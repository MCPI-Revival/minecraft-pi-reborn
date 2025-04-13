#include <libreborn/patch.h>

#include "internal.h"

// Track Whether The Connected Server Is Enhanced
static void StartGamePacket_write_injection(StartGamePacket_write_t original, StartGamePacket *self, RakNet_BitStream *bit_stream) {
    // Call Original Method
    original(self, bit_stream);
    // Add Extra Data
    uchar x = 1;
    bit_stream->Write_uchar(&x);
}
bool _server_using_improved_loading;
static void StartGamePacket_read_injection(StartGamePacket_read_t original, StartGamePacket *self, RakNet_BitStream *bit_stream) {
    // Call Original Method
    original(self, bit_stream);
    // Check If Packet Contains Extra Data
    uchar x;
    _server_using_improved_loading = bit_stream->Read_uchar(&x);
    _multiplayer_clear_updates();
}

// "Special" Version Of RequestChunkPacket That Sends Light Data
bool _request_full_chunk = false;
static void negate_int(int &x) {
    x = -x - 1;
}
static void ClientSideNetworkHandler_requestNextChunk_RakNetInstance_send_injection(RakNetInstance *self, Packet &packet) {
    if (_request_full_chunk) {
        RequestChunkPacket *data = (RequestChunkPacket *) &packet;
        negate_int(data->x);
        negate_int(data->z);
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

// Handle Requested Chunk Data
bool _multiplayer_is_loading_chunks(const ClientSideNetworkHandler *self) {
    return _server_using_improved_loading && self->level == nullptr;
}
static void ClientSideNetworkHandler_handle_ChunkDataPacket_injection(ClientSideNetworkHandler_handle_ChunkDataPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, ChunkDataPacket *packet) {
    if (_multiplayer_is_loading_chunks(self)) {
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
        _multiplayer_chunk_received(chunk);
        // Request Next Chunk
        _request_full_chunk = true;
        self->requestNextChunk();
    } else {
        // Call Original Method
        original(self, rak_net_guid, packet);
    }
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
    overwrite_calls(StartGamePacket_write, StartGamePacket_write_injection);
    overwrite_calls(StartGamePacket_read, StartGamePacket_read_injection);

    // Send Entire Chunk Data
    overwrite_call((void *) 0x6d72c, RakNetInstance_send, ClientSideNetworkHandler_requestNextChunk_RakNetInstance_send_injection);
    overwrite_calls(ServerSideNetworkHandler_handle_RequestChunkPacket, ServerSideNetworkHandler_handle_RequestChunkPacket_injection);
    overwrite_calls(ClientSideNetworkHandler_handle_ChunkDataPacket, ClientSideNetworkHandler_handle_ChunkDataPacket_injection);

    // Buffer Block Updates
    overwrite_calls(ClientSideNetworkHandler_handle_UpdateBlockPacket,  ClientSideNetworkHandler_handle_UpdateBlockPacket_injection);

    // Modify ChunkDataPacket To Always Send The Full Chunk
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x717c4, nop_patch);
    // TODO is this all needed?
    unsigned char mov_r3_ff[4] = {0xff, 0x30, 0xa0, 0xe3}; // "mov r3, #0xff"
    patch((void *) 0x7178c, mov_r3_ff);
}