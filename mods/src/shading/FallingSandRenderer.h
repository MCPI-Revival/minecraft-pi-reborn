#pragma once

#include <symbols/minecraft.h>

// Render Falling Sand
struct FallingSandRenderer {
    // Custom Level Source
    struct SandLevelSource final : CustomLevelSource {
        // Properties
        Level *level = nullptr;
        int real_x = 0;
        int real_y = 0;
        int real_z = 0;
        int id = 0;
        int data = 0;
        float brightness = 0;
        // Methods
        int getTile(int x, int y, int z) override;
        int getData(int x, int y, int z) override;
        const Material *getMaterial(int x, int y, int z) override;
        float getBrightness(int x, int y, int z) override;
    };

    // Properties
    SandLevelSource level_source;
    TileRenderer *renderer;

    // Methods
    FallingSandRenderer();
    [[nodiscard]] bool render() const;
};
MCPI_INTERNAL FallingSandRenderer *get_falling_sand_renderer();