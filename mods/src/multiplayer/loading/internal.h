#pragma once

#include <array>

#include <symbols/ClientSideNetworkHandler.h>
#include <symbols/ServerSideNetworkHandler.h>
#include <symbols/Minecraft.h>
#include <symbols/DisconnectionScreen.h>
#include <symbols/Strings.h>
#include <symbols/ReadyPacket.h>
#include <symbols/Packet.h>
#include <symbols/Level.h>
#include <symbols/Tile.h>
#include <symbols/RakNetInstance.h>
#include <symbols/StartGamePacket.h>
#include <symbols/RakNet_BitStream.h>
#include <symbols/RequestChunkPacket.h>
#include <symbols/ChunkSource.h>
#include <symbols/LevelChunk.h>
#include <symbols/RakNet_AddressOrGUID.h>
#include <symbols/RakNet_RakPeer.h>
#include <symbols/ChunkDataPacket.h>
#include <symbols/UpdateBlockPacket.h>
#include <symbols/RandomLevelSource.h>
#include <symbols/BiomeSource.h>
#include <symbols/CThread.h>

#include <mods/misc/misc.h>
#include <mods/common.h>

#include "../internal.h"

// Init
#define init(name) MCPI_INTERNAL void _init_multiplayer_loading_##name()
init(packets);
init(thread);
init(misc);
init(terrain);
#undef init

// Track Whether Improved Chunk Loading Is Active
// Only Access This Value If You Know You're Joining A Server!
MCPI_INTERNAL extern bool _server_using_improved_loading;
// Request Full Chunk Data
MCPI_INTERNAL extern bool _request_full_chunk;
// Disable Terrain Generation
MCPI_INTERNAL extern bool _inhibit_terrain_generation;

// Stop Thread
MCPI_INTERNAL void _multiplayer_stop_thread(Minecraft *minecraft);
// Check If Network Handler Is Loading Chunks
MCPI_INTERNAL bool _multiplayer_is_loading_chunks(const ClientSideNetworkHandler *self);

// Buffer Block Updates
MCPI_INTERNAL void _multiplayer_clear_updates();
MCPI_INTERNAL void _multiplayer_set_tile(int x, int y, int z, int tile_id, int data);

// Chunk Data Structure
struct ChunkData {
    // Constants
    static constexpr int SIZE = LevelSize::CHUNK_SIZE;
    static constexpr int COLUMNS = SIZE * SIZE;
    static constexpr int HEIGHT = LevelSize::HEIGHT;
    static constexpr int TOTAL_SIZE = COLUMNS * HEIGHT;
    static constexpr int TOTAL_SIZE_HALF = TOTAL_SIZE / 2;
    static constexpr int WORLD_SIZE = world_size; // In Chunks
    // Fields
    int x;
    int z;
    struct Raw {
        std::array<unsigned char, TOTAL_SIZE> blocks;
        std::array<unsigned char, TOTAL_SIZE_HALF> data;
        std::array<unsigned char, TOTAL_SIZE_HALF> light_sky;
        std::array<unsigned char, TOTAL_SIZE_HALF> light_block;
    } raw;
};

// Receive Chunk Data
MCPI_INTERNAL void _multiplayer_chunk_received(ChunkData *data);