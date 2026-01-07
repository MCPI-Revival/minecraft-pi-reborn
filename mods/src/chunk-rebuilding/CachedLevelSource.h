#pragma once

#include <symbols/LevelSource.h>

// Cache Chunk Information
struct CachedLevelSource final : CustomLevelSource {
    // Constants
    static constexpr int BORDER = 1;
    static constexpr int MAX_SIZE = 16 + (BORDER * 2);
    static constexpr int MIN_Y = 0;
    static constexpr int MAX_Y = 127;
    static constexpr int MAX_BRIGHTNESS = 15;
    static constexpr unsigned int LEVEL_CHUNK_SIZE = 16;
    static constexpr int MAX_LEVEL_CHUNKS = (MAX_SIZE + (2 * (LEVEL_CHUNK_SIZE - 1))) / LEVEL_CHUNK_SIZE;

    // Properties
    struct Data {
        int id = 0;
        int data = 0;
        bool is_empty = true;
        int raw_brightness = 0;
        float brightness = 0;
        const Material *material = nullptr;
        bool should_render = false;
    } data[MAX_SIZE][MAX_SIZE][MAX_SIZE]; // [X][Z][Y]
    LevelChunk *chunks[MAX_LEVEL_CHUNKS][MAX_LEVEL_CHUNKS] = {};
    bool should_render = false;
    bool touched_sky = false;
    float light_ramp[MAX_BRIGHTNESS + 1] = {};
    int level_sky_darken = 0;

    // Region
    int x0 = 0;
    int y0 = 0;
    int z0 = 0;
    int x1 = 0;
    int y1 = 0;
    int z1 = 0;
    int chunk_x0 = 0;
    int chunk_z0 = 0;
    int chunks_x = 0;
    int chunks_z = 0;

    // Store Properties
    void _prepare_cache(Level *level, int x0_, int y0_, int z0_, int x1_, int y1_, int z1_);
    static LevelChunk *_clone(LevelChunk *chunk);
    void _copy_chunks(Level *level);
    void _cache();
    void _copy_tiles();
    void _check_if_should_render();
    int _get_raw_brightness(LevelChunk *chunk, int x, int y, int z);
    [[nodiscard]] int _get_raw_brightness(int x, int y, int z, bool param_1) const;

    // Get Properties
    int getTile(int x, int y, int z) override;
    int getData(int x, int y, int z) override;
    bool isEmptyTile(int x, int y, int z) override;
    float getBrightness(int x, int y, int z) override;
    const Material *getMaterial(int x, int y, int z) override;
    [[nodiscard]] bool _should_render(int x, int y, int z) const;
};