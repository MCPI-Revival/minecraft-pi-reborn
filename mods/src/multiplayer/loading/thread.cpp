#include <queue>
#include <pthread.h>

#include <libreborn/patch.h>

#include "internal.h"

// Track Progress
static bool update_progress(Minecraft *minecraft) {
    // Count Loaded Chunks
    ClientSideNetworkHandler *handler = (ClientSideNetworkHandler *) minecraft->network_handler;
    float total = 0;
    float loaded = 0;
    for (const bool x : handler->chunk_loaded) {
        total++;
        if (x) {
            loaded++;
        }
    }
    // Update Progress
    minecraft->progress_state = 1;
    minecraft->progress = int((loaded / total) * 100);
    // Return
    return total != loaded;
}

// Handle Stopping Thread
static volatile bool should_stop_thread;
void _multiplayer_stop_thread(Minecraft *minecraft) {
    should_stop_thread = true;
    CThread *&thread = minecraft->level_generation_thread;
    if (thread) {
        thread->destructor_deleting();
        thread = nullptr;
    }
}

// Empty Chunks
static void create_chunks(Minecraft *minecraft, const Level *level) {
    // Set Progress
    minecraft->progress_state = 2;
    minecraft->progress = 0;

    // Create Empty Chunks
    constexpr float total = ChunkData::WORLD_SIZE * ChunkData::WORLD_SIZE;
    float created = 0;
    _inhibit_terrain_generation = true;
    for (int x = 0; x < ChunkData::WORLD_SIZE; x++) {
        for (int z = 0; z < ChunkData::WORLD_SIZE; z++) {
            // Create Chunk
            LevelChunk *chunk = level->chunk_source->create(x, z);
            if (!chunk || chunk->isEmpty()) {
                IMPOSSIBLE();
            }
            memset(chunk->blocks, 0, ChunkData::TOTAL_SIZE);
            created++;

            // Mark As Custom
            chunk->should_save = true;
            chunk->loaded_from_disk = true;
            for (uchar &mask : chunk->dirty_columns) {
                mask = 0xff;
            }

            // Update Progress
            minecraft->progress = int((created / total) * 100);
        }
    }
    _inhibit_terrain_generation = false;
}

// Thread For Processing Data
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static std::queue<ChunkData *> chunks;
static void *process_thread(void *thread_data) {
    // Create Empty Chunks
    Minecraft *minecraft = (Minecraft *) thread_data;
    Level *level = minecraft->level;
    create_chunks(minecraft, level);

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

        // Construct Tile Entities
        for (int x = 0; x < ChunkData::SIZE; x++) {
            for (int y = 0; y < ChunkData::HEIGHT; y++) {
                for (int z = 0; z < ChunkData::SIZE; z++) {
                    const int tile_id = level_chunk->getTile(x, y, z);
                    if (tile_id != 0 && Tile::isEntityTile[tile_id]) {
                        level_chunk->getTileEntity(x, y, z);
                    }
                }
            }
        }

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
    minecraft->progress_state = 3;
    minecraft->progress = -1;
    level->saveGame();
    level->chunk_source->saveAll(false);
    level->prepare();

    // Done Loading
    minecraft->progress_state = 0;
    minecraft->generating_level = false;
    return nullptr;
}

// Handle Requested Chunk Data
void _multiplayer_chunk_received(ChunkData *data) {
    pthread_mutex_lock(&mutex);
    chunks.push(data);
    pthread_mutex_unlock(&mutex);
}

// Request Chunks
static void start_requesting_chunks(const Minecraft *minecraft) {
    // Each response packet will in turn create a new request.
    // So X requests will give X responses, which will create another X requests.
    constexpr int CHUNK_LOADING_PARALLELISM = 16;
    ClientSideNetworkHandler *handler = (ClientSideNetworkHandler *) minecraft->network_handler;
    for (int i = 0; i < CHUNK_LOADING_PARALLELISM; i++) {
        _request_full_chunk = true;
        handler->requestNextChunk();
    }
}

// Disable Client-Side Terrain Generation
static CThread *Minecraft_setLevel_CThread_injection(CThread *self, void *func, void *data) {
    const Minecraft *minecraft = (Minecraft *) data;
    const Level *level = minecraft->level;
    if (level->is_client_side && _server_using_improved_loading) {
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

// Init
void _init_multiplayer_loading_thread() {
    overwrite_call((void *) 0x152cc, CThread_constructor, Minecraft_setLevel_CThread_injection);
}