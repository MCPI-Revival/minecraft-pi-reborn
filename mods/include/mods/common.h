#pragma once

// Level Dimensions
namespace LevelSize {
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_COUNT = 16;
    static constexpr int SIZE = CHUNK_COUNT * CHUNK_SIZE;
    static constexpr int HEIGHT = 128;
}

// The game's various texture atlases.
namespace AtlasSize {
    static constexpr int TILE_SIZE = 16;
    static constexpr int TILE_COUNT = 16;
    static constexpr int SIZE = TILE_COUNT * TILE_SIZE;
}

// The division between TileItems and real Items.
static constexpr int TILE_ITEM_BARRIER = 256;