#pragma once

#include <array>

#include <symbols/minecraft.h>

#include "../internal.h"

// Init
#define init(name) __attribute__((visibility("internal"))) void _init_multiplayer_loading_##name()
init(packets);
init(thread);
init(misc);
init(terrain);
#undef init

// Track Whether Improved Chunk Loading Is Active
// Only Access This Value If You Know You're Joining A Server!
__attribute__((visibility("internal"))) extern bool _server_using_improved_loading;
// Request Full Chunk Data
__attribute__((visibility("internal"))) extern bool _request_full_chunk;
// Disable Terrain Generation
__attribute__((visibility("internal"))) extern bool _inhibit_terrain_generation;

// Stop Thread
__attribute__((visibility("internal"))) void _multiplayer_stop_thread(Minecraft *minecraft);
// Check If Network Handler Is Loading Chunks
__attribute__((visibility("internal"))) bool _multiplayer_is_loading_chunks(const ClientSideNetworkHandler *self);

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

// Receive Chunk Data
__attribute__((visibility("internal"))) void _multiplayer_chunk_received(ChunkData *data);