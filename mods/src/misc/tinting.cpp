#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>

#include "misc-internal.h"

// Change Grass Color
static int32_t get_color(LevelSource *level_source, int32_t x, int32_t z) {
    // Get Biome
    const Biome *biome = level_source->getBiome(x, z);
    if (biome == nullptr) {
        return 0;
    }
    // Return
    return biome->color;
}
#define BIOME_BLEND_SIZE 7
static int32_t GrassTile_getColor_injection(__attribute__((unused)) GrassTile_getColor_t original, __attribute__((unused)) GrassTile *tile, LevelSource *level_source, const int32_t x, __attribute__((unused)) int32_t y, const int32_t z) {
    int r_sum = 0;
    int g_sum = 0;
    int b_sum = 0;
    int color_sum = 0;
    const int x_start = x - (BIOME_BLEND_SIZE / 2);
    const int z_start = z - (BIOME_BLEND_SIZE / 2);
    for (int x_offset = 0; x_offset < BIOME_BLEND_SIZE; x_offset++) {
        for (int z_offset = 0; z_offset < BIOME_BLEND_SIZE; z_offset++) {
            const int32_t color = get_color(level_source, x_start + x_offset, z_start + z_offset);
            r_sum += (color >> 16) & 0xff;
            g_sum += (color >> 8) & 0xff;
            b_sum += color & 0xff;
            color_sum++;
        }
    }
    const int r_avg = r_sum / color_sum;
    const int g_avg = g_sum / color_sum;
    const int b_avg = b_sum / color_sum;
    return (r_avg << 16) | (g_avg << 8) | b_avg;
}
static int32_t TallGrass_getColor_injection(TallGrass_getColor_t original, TallGrass *tile, LevelSource *level_source, const int32_t x, const int32_t y, const int32_t z) {
    const int32_t original_color = original(tile, level_source, x, y, z);
    if (original_color == 0x339933) {
        return GrassTile_getColor_injection(nullptr, nullptr, level_source, x, y, z);
    } else {
        return original_color;
    }
}

// No Block Tinting
template <typename T>
static int32_t Tile_getColor_injection(__attribute__((unused)) std::function<int(T *, LevelSource *, int, int, int)> original, __attribute__((unused)) T *self, __attribute__((unused)) LevelSource *level_source, __attribute__((unused)) int x, __attribute__((unused)) int y, __attribute__((unused)) int z) {
    return 0xffffff;
}

// Init
void _init_misc_tinting() {
    // Change Grass Color
    if (feature_has("Add Biome Colors To Grass", server_disabled)) {
        overwrite_calls(GrassTile_getColor, GrassTile_getColor_injection);
        overwrite_calls(TallGrass_getColor, TallGrass_getColor_injection);
    }

    // Disable Block Tinting
    if (feature_has("Disable Block Tinting", server_disabled)) {
        overwrite_calls(GrassTile_getColor, Tile_getColor_injection<GrassTile>);
        overwrite_calls(TallGrass_getColor, Tile_getColor_injection<TallGrass>);
        overwrite_calls(StemTile_getColor, Tile_getColor_injection<StemTile>);
        overwrite_calls(LeafTile_getColor, Tile_getColor_injection<LeafTile>);
        overwrite_calls(LiquidTile_getColor, Tile_getColor_injection<LiquidTile>);
    }
}