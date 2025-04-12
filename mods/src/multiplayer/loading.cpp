#include <algorithm>
#include <queue>
#include <array>
#include <pthread.h>

#include <symbols/minecraft.h>
#include <libreborn/patch.h>
#include <mods/misc/misc.h>

#include "internal.h"

// Track Whether The Connected Server Is Enhanced
static void StartGamePacket_write_injection(StartGamePacket_write_t original, StartGamePacket *self, RakNet_BitStream *bit_stream) {
    // Call Original Method
    original(self, bit_stream);
    // Add Extra Data
    uchar x = 1;
    bit_stream->Write_uchar(&x);
}
static bool server_using_improved_loading; // Only Access This Value If You Know You're Joining A Server!
static void StartGamePacket_read_injection(StartGamePacket_read_t original, StartGamePacket *self, RakNet_BitStream *bit_stream) {
    // Call Original Method
    original(self, bit_stream);
    // Check If Packet Contains Extra Data
    uchar x;
    server_using_improved_loading = bit_stream->Read_uchar(&x);
}

// Chunk Data Structure
struct ChunkData {
    // Constants
    static constexpr int SIZE = 16;
    static constexpr int COLUMNS = SIZE * SIZE;
    static constexpr int HEIGHT = 128;
    static constexpr int TOTAL_SIZE = COLUMNS * HEIGHT;
    static constexpr int TOTAL_SIZE_HALF = TOTAL_SIZE / 2;
    static constexpr int WORLD_SIZE = 16; // In Chunks
    // Fields
    int x;
    int z;
    std::array<unsigned char, TOTAL_SIZE> blocks;
    std::array<unsigned char, TOTAL_SIZE_HALF> data;
    std::array<unsigned char, TOTAL_SIZE_HALF> light_sky;
    std::array<unsigned char, TOTAL_SIZE_HALF> light_block;
};

// "Special" Version Of RequestChunkPacket That Sends Light Data
static bool request_full_chunk = false;
static void negate_int(int &x) {
    x = -x - 1;
}
static void ClientSideNetworkHandler_requestNextChunk_RakNetInstance_send_injection(RakNetInstance *self, Packet &packet) {
    if (request_full_chunk) {
        RequestChunkPacket *data = (RequestChunkPacket *) &packet;
        negate_int(data->x);
        negate_int(data->z);
        request_full_chunk = false;
    }
    self->send(packet);
}
static void ServerSideNetworkHandler_handle_RequestChunkPacket_injection(ServerSideNetworkHandler_handle_RequestChunkPacket_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, RequestChunkPacket *packet) {
    if (packet->x >= 0) {
        // Normal Packet, Call Original Method
        original(self, rak_net_guid, packet);
        return;
    }

    // Get Chunk
    negate_int(packet->x);
    negate_int(packet->z);
    if (!self->level) {
        return;
    }
    const LevelChunk *chunk = self->level->chunk_source->getChunk(packet->x, packet->z);
    if (!chunk) {
        return;
    }

    // Manually Create ChunkDataPacket
    RakNet_BitStream *stream = RakNet_BitStream::allocate();
    stream->constructor();
    uchar id = 0x9e;
    stream->Write_uchar(&id);
    stream->Write_int(&packet->x);
    stream->Write_int(&packet->z);
    stream->Write_bytes(chunk->blocks, ChunkData::TOTAL_SIZE);
    stream->Write_bytes(chunk->data.data, ChunkData::TOTAL_SIZE_HALF);
    stream->Write_bytes(chunk->light_sky.data, ChunkData::TOTAL_SIZE_HALF);
    stream->Write_bytes(chunk->light_block.data, ChunkData::TOTAL_SIZE_HALF);

    // Send Packet
    RakNet_AddressOrGUID target;
    target.constructor(rak_net_guid);
    self->peer->Send(stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, &target, false, 0);
    stream->destructor(0);
    ::operator delete(stream);
}

// Request Chunks
static void start_requesting_chunks(const Minecraft *minecraft) {
    // Each response packet will in turn create a new request.
    // So X requests will give X responses, which will create another X requests.
    constexpr int CHUNK_LOADING_PARALLELISM = 16;
    ClientSideNetworkHandler *handler = (ClientSideNetworkHandler *) minecraft->network_handler;
    for (int i = 0; i < CHUNK_LOADING_PARALLELISM; i++) {
        request_full_chunk = true;
        handler->requestNextChunk();
    }
}

// Prevent Terrain Generation When Accessing Chunks
static bool inhibit_terrain_generation = false;
template <typename... Args>
static void block_terrain_generation_injection(const std::function<void(Args...)> &original, Args... args) {
    if (inhibit_terrain_generation) {
        return;
    }
    // Call Original Method
    original(std::forward<Args>(args)...);
}
static LevelChunk *RandomLevelSource_getChunk_injection(RandomLevelSource_getChunk_t original, RandomLevelSource *self, const int chunk_x, const int chunk_z) {
    // Call Original Method
    LevelChunk *ret = original(self, chunk_x, chunk_z);
    // Mark Terrain As Generated
    if (inhibit_terrain_generation) {
        ret->done_generating = true;
    }
    // Return
    return ret;
}

// Track Progress
static bool update_progress(Minecraft *minecraft) {
    // Count loaded Chunks
    ClientSideNetworkHandler *handler = (ClientSideNetworkHandler *) minecraft->network_handler;
    float total = 0;
    float loaded = 0;
    for (const bool x : handler->chunk_loaded) {
        total++;
        if (x) {
            loaded++;
        }
    }
    // Update
    if (total != loaded) {
        minecraft->progress_state = 1;
        minecraft->progress = int((loaded / total) * 100);
        return true;
    } else {
        // Done Loading
        return false;
    }
}

// Thread For Processing Data
static volatile bool should_stop_thread;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static std::queue<ChunkData *> chunks;
static void *process_thread(void *thread_data) {
    // Set Progress
    Minecraft *minecraft = (Minecraft *) thread_data;
    update_progress(minecraft);

    // Create Empty Chunks
    Level *level = minecraft->level;
    inhibit_terrain_generation = true;
    for (int x = 0; x < ChunkData::WORLD_SIZE; x++) {
        for (int z = 0; z < ChunkData::WORLD_SIZE; z++) {
            // Create Chunk
            LevelChunk *chunk = level->chunk_source->create(x, z); // What is this doing?
            if (!chunk || chunk->isEmpty()) {
                IMPOSSIBLE();
            }
            memset(chunk->blocks, 0, ChunkData::TOTAL_SIZE);
            // Mark As Custom
            chunk->should_save = true;
            chunk->loaded_from_disk = true;
            for (uchar &mask : chunk->dirty_columns) {
                mask = 0xff;
            }
        }
    }
    inhibit_terrain_generation = false;

    // Load Chunks
    ClientSideNetworkHandler *handler = (ClientSideNetworkHandler *) minecraft->network_handler;
    while (update_progress(minecraft) && !should_stop_thread) {
        // Get Chunk Data
        ChunkData *chunk = nullptr;
        pthread_mutex_lock(&mutex);
        if (!chunks.empty()) {
            chunk = chunks.front();
            chunks.pop();
        }
        pthread_mutex_unlock(&mutex);
        if (!chunk) {
            // Wait
            continue;
        }

        // Fix Invalid Tiles
        for (unsigned char &tile : chunk->blocks) {
            tile = Tile::transformToValidBlockId(tile, 0, 0, 0);
        }

        // Set Chunk Data
        LevelChunk *level_chunk = level->chunk_source->create(chunk->x, chunk->z);
#define copy(a, b) memcpy(level_chunk->a, chunk->b.data(), chunk->b.size())
        copy(blocks, blocks);
        copy(data.data, data);
        copy(light_sky.data, light_sky);
        copy(light_block.data, light_block);
#undef copy

        // Post-Processing
        level_chunk->recalcHeightmapOnly();
        while (level->updateLights()) {}

        // Finished
        handler->chunk_loaded[(chunk->x * ChunkData::WORLD_SIZE) + chunk->z] = true;
        delete chunk;
    }

    // Free Queued Chunks
    pthread_mutex_lock(&mutex);
    while (!chunks.empty()) {
        delete chunks.front();
        chunks.pop();
    }
    pthread_mutex_unlock(&mutex);

    // Save Level
    level->saveGame();
    level->chunk_source->saveAll(false);

    // Done Loading
    minecraft->progress_state = 0;
    minecraft->progress = -1;
    minecraft->generating_level = false;
    return nullptr;
}

// Handle Requested Chunk Data
static bool is_loading_chunks(const ClientSideNetworkHandler *self) {
    return server_using_improved_loading && self->level == nullptr;
}
static void ClientSideNetworkHandler_handle_ChunkDataPacket_injection(ClientSideNetworkHandler_handle_ChunkDataPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, ChunkDataPacket *packet) {
    if (is_loading_chunks(self)) {
        // Improved Chunk Loading
        ChunkData *chunk = new ChunkData;

        // Parse Packet
        chunk->x = packet->x;
        chunk->z = packet->z;
#define read(type) packet->data.Read_bytes(chunk->type.data(), chunk->type.size())
        read(blocks);
        read(data);
        read(light_sky);
        read(light_block);
#undef read

        // Add Chunk To Queue
        pthread_mutex_lock(&mutex);
        chunks.push(chunk);
        pthread_mutex_unlock(&mutex);
        // Request Next Chunk
        request_full_chunk = true;
        self->requestNextChunk();
    } else {
        // Call Original Method
        original(self, rak_net_guid, packet);
    }
}

// Disable Client-Side Terrain Generation
static CThread *Minecraft_setLevel_CThread_injection(CThread *self, void *func, void *data) {
    const Minecraft *minecraft = (Minecraft *) data;
    const Level *level = minecraft->level;
    if (level->is_client_side && server_using_improved_loading) {
        // Request Chunks
        if (!chunks.empty()) {
            IMPOSSIBLE();
        }
        should_stop_thread = false;
        func = (void *) process_thread;
        start_requesting_chunks(minecraft);
    }
    // Call Original Method
    return self->constructor(func, data);
}

// Handle Disconnection While Loading
static bool should_disconnect = false;
static void ClientSideNetworkHandler_onDisconnect_injection(ClientSideNetworkHandler_onDisconnect_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid) {
    // Call Original Method
    original(self, rak_net_guid);

    // Handle Stopping loading
    if (is_loading_chunks(self)) {
        should_disconnect = true;
    }
}
static void handle_disconnect_tick(Minecraft *minecraft) {
    if (!should_disconnect) {
        return;
    }
    should_disconnect = false;

    // Stop Thread
    should_stop_thread = true;
    CThread *&thread = minecraft->level_generation_thread;
    if (thread) {
        thread->destructor_deleting();
        thread = nullptr;
    }
    minecraft->level_generation_signal = true;

    // Leave Game
    minecraft->leaveGame(false); // This Destroys Self!
    DisconnectionScreen *screen = DisconnectionScreen::allocate();
    screen->constructor(Strings::unable_to_connect);
    minecraft->setScreen((Screen *) screen);
}

// Init
void _init_multiplayer_loading() {
    // Modify ChunkDataPacket To Always Send The Full Chunk
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x717c4, nop_patch);
    // TODO is this all needed?
    unsigned char mov_r3_ff[4] = {0xff, 0x30, 0xa0, 0xe3}; // "mov r3, #0xff"
    patch((void *) 0x7178c, mov_r3_ff);

    // Detect Modded Servers
    overwrite_calls(StartGamePacket_write, StartGamePacket_write_injection);
    overwrite_calls(StartGamePacket_read, StartGamePacket_read_injection);

    // Send Entire Chunk Data
    overwrite_call((void *) 0x6d72c, RakNetInstance_send, ClientSideNetworkHandler_requestNextChunk_RakNetInstance_send_injection);
    overwrite_calls(ServerSideNetworkHandler_handle_RequestChunkPacket, ServerSideNetworkHandler_handle_RequestChunkPacket_injection);

    // Request Chunks Instead Of Generating Them
    overwrite_call((void *) 0x152cc, CThread_constructor, Minecraft_setLevel_CThread_injection);
    overwrite_calls(ClientSideNetworkHandler_handle_ChunkDataPacket, ClientSideNetworkHandler_handle_ChunkDataPacket_injection);

    // Allow Blocking Terrain Generation
    overwrite_calls(RandomLevelSource_buildSurface, block_terrain_generation_injection<RandomLevelSource *, int, int, uchar *, Biome **>);
    overwrite_calls(RandomLevelSource_prepareHeights, block_terrain_generation_injection<RandomLevelSource *, int, int, uchar *, void *, float *>);
    overwrite_calls(LevelChunk_recalcHeightmap, block_terrain_generation_injection<LevelChunk *>);
    overwrite_calls(RandomLevelSource_getChunk, RandomLevelSource_getChunk_injection);

    // Handle Disconnection
    overwrite_calls(ClientSideNetworkHandler_onDisconnect, ClientSideNetworkHandler_onDisconnect_injection);
    misc_run_on_tick(handle_disconnect_tick);
}