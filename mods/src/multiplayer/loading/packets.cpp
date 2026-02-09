#include "internal.h"

#include <libreborn/patch.h>

#include "stb_image.h"
#include "stb_image_write.h"
STBIWDEF unsigned char *stbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality);

#include <mods/multiplayer/packets.h>

// Track Whether The Connected Server Is Enhanced
static constexpr int flag = StartGameFlags::USE_IMPROVED_LOADING;
bool _server_using_improved_loading;
static void StartGamePacket_handle_injection(StartGamePacket_handle_t original, StartGamePacket *self, const RakNet_RakNetGUID &guid, NetEventCallback *callback) {
    // Check If The Packet Contains Extra Data
    _server_using_improved_loading = StartGameFlags::has_received_from_server(flag);
    _multiplayer_clear_updates();
    // Call Original Method
    original(self, guid, callback);
}

// "Special" Version Of RequestChunkPacket That Sends Light Data
bool _request_full_chunk = false;
void multiplayer_negate(int &x) {
    x = -x - 1;
}
static void ClientSideNetworkHandler_requestNextChunk_RakNetInstance_send_injection(RakNetInstance *self, Packet &packet) {
    if (_request_full_chunk) {
        RequestChunkPacket *data = (RequestChunkPacket *) &packet;
        multiplayer_negate(data->x);
        multiplayer_negate(data->z);
        _request_full_chunk = false;
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
    multiplayer_negate(packet->x);
    multiplayer_negate(packet->z);
    if (!self->level) {
        return;
    }
    const LevelChunk *chunk = self->level->chunk_source->getChunk(packet->x, packet->z);
    if (!chunk) {
        return;
    }
    multiplayer_negate(packet->x);
    multiplayer_negate(packet->z);

    // Compress Data
    static ChunkData::Raw data;
#define copy(src, dest) memcpy(data.dest.data(), chunk->src, data.dest.size())
    copy(blocks, blocks);
    copy(data.data, data);
    copy(light_sky.data, light_sky);
    copy(light_block.data, light_block);
#undef copy
    int compressed_len;
    unsigned char *compressed = stbi_zlib_compress((uchar *) &data, sizeof(data), &compressed_len, stbi_write_png_compression_level);

    // Manually Create ChunkDataPacket
    RakNet_BitStream *stream = RakNet_BitStream::allocate();
    stream->constructor();
    uchar id = 0x9e;
    stream->Write_uchar(&id);
    stream->Write_int(&packet->x);
    stream->Write_int(&packet->z);
    stream->Write_int(&compressed_len);
    stream->Write_bytes(compressed, compressed_len);

    // Send Packet
    RakNet_AddressOrGUID target = {};
    target.constructor(rak_net_guid);
    self->peer->Send(stream, HIGH_PRIORITY, RELIABLE, 0, &target, false, 0);

    // Free
    free(compressed);
    stream->destructor(0);
    ::operator delete(stream);
}

// Handle Requested Chunk Data
bool _multiplayer_is_loading_chunks(const ClientSideNetworkHandler *self) {
    return _server_using_improved_loading && self->level == nullptr;
}
static void ClientSideNetworkHandler_handle_ChunkDataPacket_injection(ClientSideNetworkHandler_handle_ChunkDataPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, ChunkDataPacket *packet) {
    if (!_multiplayer_is_loading_chunks(self)) {
        // Expecting A Normal/Vanilla Packet
        if (packet->x < 0) {
            ERR("Unexpected Enhanced ChunkDataPacket");
        }
        // Call Original Method
        original(self, rak_net_guid, packet);
        return;
    }

    // Expecting A Custom/Enhanced Packet
    if (packet->x >= 0) {
        ERR("Unexpected Vanilla ChunkDataPacket");
    }
    multiplayer_negate(packet->x);
    multiplayer_negate(packet->z);

    // Improved Chunk Loading
    ChunkData *chunk = new ChunkData;

    // Parse Packet
    chunk->x = packet->x;
    chunk->z = packet->z;
    int compressed_len;
    bool success = packet->data.Read_int(&compressed_len);
    if (success) {
        uchar *compressed = new uchar[compressed_len];
        success = packet->data.Read_bytes(compressed, compressed_len);
        if (success) {
            ChunkData::Raw &out = chunk->raw;
            stbi_zlib_decode_buffer((char *) &out, sizeof(out), (const char *) compressed, compressed_len);
        }
        delete[] compressed;
    }

    // Add Chunk To Queue
    _multiplayer_chunk_received(chunk);
    // Request Next Chunk
    _request_full_chunk = true;
    self->requestNextChunk();
}

// Buffer Block Updates Received While Chunks Are Loading
static void ClientSideNetworkHandler_handle_UpdateBlockPacket_injection(ClientSideNetworkHandler_handle_UpdateBlockPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, UpdateBlockPacket *packet) {
    if (_multiplayer_is_loading_chunks(self)) {
        // Improved Chunk Loading
        _multiplayer_set_tile(packet->x, packet->y, packet->z, packet->tile_id, packet->data);
    } else {
        // Call Original Method
        original(self, rak_net_guid, packet);
    }
}

// Init
void _init_multiplayer_loading_packets() {
    // Detect Modded Servers
    StartGameFlags::send_to_clients(flag);
    overwrite_calls(StartGamePacket_handle, StartGamePacket_handle_injection);

    // Send Entire Chunk Data
    overwrite_call((void *) 0x6d72c, RakNetInstance_send, ClientSideNetworkHandler_requestNextChunk_RakNetInstance_send_injection);
    overwrite_calls(ServerSideNetworkHandler_handle_RequestChunkPacket, ServerSideNetworkHandler_handle_RequestChunkPacket_injection);
    overwrite_calls(ClientSideNetworkHandler_handle_ChunkDataPacket, ClientSideNetworkHandler_handle_ChunkDataPacket_injection);

    // Buffer Block Updates
    overwrite_calls(ClientSideNetworkHandler_handle_UpdateBlockPacket,  ClientSideNetworkHandler_handle_UpdateBlockPacket_injection);

    // Modify ChunkDataPacket To Always Send The Full Chunk
    unsigned char mov_r3_ff[4] = {0xff, 0x30, 0xa0, 0xe3}; // "mov r3, #0xff"
    patch((void *) 0x7178c, mov_r3_ff);
}