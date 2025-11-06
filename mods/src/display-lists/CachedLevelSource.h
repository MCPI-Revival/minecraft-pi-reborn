#pragma once

#include <symbols/minecraft.h>

// Cache Chunk Information
struct CachedLevelSource final : CustomLevelSource {
    // Constants
    static constexpr int BORDER = 1;
    static constexpr int MAX_SIZE = 16 + (BORDER * 2);
    static constexpr int MIN_Y = 0;
    static constexpr int MAX_Y = 127;

    // Properties
    LevelChunk *chunks[MAX_SIZE][MAX_SIZE] = {};
    struct Data {
        int id = 0;
        int data = 0;
        bool is_empty = true;
        int raw_brightness = 0;
        float brightness = 0;
        const Material *material = nullptr;
        bool should_render = false;
    } data[MAX_SIZE][MAX_SIZE][MAX_SIZE]; // [X][Z][Y]
    bool should_render = false;
    Level *level = nullptr;

    // Region
    int x0 = 0;
    int y0 = 0;
    int z0 = 0;
    int x1 = 0;
    int y1 = 0;
    int z1 = 0;

    // Store Properties
    void _cache(int x0_, int y0_, int z0_, int x1_, int y1_, int z1_);
    void _copy_tiles();
    void _check_if_should_render();
    [[nodiscard]] int _get_raw_brightness(int x, int y, int z, bool param_1) const;

    // Get Properties
    int getTile(int x, int y, int z) override;
    int getData(int x, int y, int z) override;
    bool isEmptyTile(int x, int y, int z) override;
    float getBrightness(int x, int y, int z) override;
    const Material *getMaterial(int x, int y, int z) override;
    [[nodiscard]] bool _should_render(int x, int y, int z) const;
};