#include <unordered_set>

#include <libreborn/patch.h>
#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/feature/feature.h>
#include <mods/shading/FallingSandRenderer.h>
#include <mods/display-lists/display-lists.h>

#include "internal.h"

// Change Grass Color
static bool grass_colors_loaded = false;
#define GRASS_COLORS_SIZE 256
static uint32_t grass_colors[GRASS_COLORS_SIZE][GRASS_COLORS_SIZE];
static void Minecraft_init_injection(Minecraft_init_t original, Minecraft *self) {
    // Call Original Method
    original(self);
    // Load
    const uint32_t id = self->textures->loadTexture("misc/grasscolor.png", true);
    const Texture *data = self->textures->getTemporaryTextureData(id);
    if (data == nullptr || data->width != GRASS_COLORS_SIZE || data->height != GRASS_COLORS_SIZE) {
        return;
    }
    const uint32_t *texture = (uint32_t *) data->data;
    // Copy
    for (int y = 0; y < GRASS_COLORS_SIZE; y++) {
        for (int x = 0; x < GRASS_COLORS_SIZE; x++) {
            grass_colors[y][x] = texture[(y * GRASS_COLORS_SIZE) + x];
        }
    }
    grass_colors_loaded = true;
}
static Level *get_level_from_source(LevelSource *source) {
    // Test Custom Level Sources
    Level *level = get_level_from_falling_sand_renderer(source);
    if (level) {
        return level;
    }
    // Test If Level Source Is A Level
    const void *vtable = source->vtable;
    static std::unordered_set level_vtables = {
        (const void *) Level::VTable::base,
        (const void *) ServerLevel::VTable::base,
        (const void *) MultiPlayerLevel::VTable::base,
        (const void *) CreatorLevel::VTable::base
    };
    if (level_vtables.contains(vtable)) {
        return (Level *) source;
    }
    // Test If Level Source Is A Region
    if (vtable == (const void *) Region::VTable::base) {
        return ((Region *) source)->level;
    }
    // Unable To Retrieve Level
    return nullptr;
}
static BiomeSource *get_biome_source_from_level_source(LevelSource *source) {
    BiomeSource *biome_source = get_biome_source_on_chunk_rebuild_thread();
    if (biome_source) {
        return biome_source;
    }
    Level *level = get_level_from_source(source);
    if (level) {
        return level->getBiomeSource();
    }
    return nullptr;
}
static int32_t GrassTile_getColor_injection(GrassTile_getColor_t original, GrassTile *tile, LevelSource *level_source, const int32_t x, const int32_t y, const int32_t z) {
    // Get Level
    BiomeSource *biome_source = get_biome_source_from_level_source(level_source);
    if (!biome_source) {
        // Call Original Method
        return original(tile, level_source, x, y, z);
    }

    // Find Biome Temperature
    biome_source->getBiomeBlock(x, z, 1, 1);
    const float temperature = biome_source->temperature[0];
    float downfall = biome_source->downfall[0];
    // Get Grass Color
    downfall *= temperature;
    const int xx = (int) ((1 - temperature) * (GRASS_COLORS_SIZE - 1));
    const int yy = (int) ((1 - downfall) * (GRASS_COLORS_SIZE - 1));
    uint32_t color = grass_colors[yy][xx];
    // Convert From ABGR To ARGB
    const uint8_t b = (color >> 16) & 0xff;
    const uint8_t g = (color >> 8)  & 0xff;
    const uint8_t r = color & 0xff;
    color = (r << 16) | (g << 8) | b;
    return (int32_t) color;
}
static int32_t TallGrass_getColor_injection(TallGrass_getColor_t original, TallGrass *tile, LevelSource *level_source, const int32_t x, const int32_t y, const int32_t z) {
    const int32_t original_color = original(tile, level_source, x, y, z);
    if (original_color == 0x339933) {
        return GrassTile_getColor_injection(nullptr, nullptr, level_source, x, y, z);
    } else {
        // Dead Bush
        return original_color;
    }
}

// Grass Side Tinting
struct GrassSideTile final : CustomTile {
    GrassSideTile(const int id, const int texture, const Material *material):
        CustomTile(id, texture, material) {}
    bool shouldRenderFace(LevelSource *level_source, const int x, const int y, const int z, const int face) override {
        return face > 1 && CustomTile::shouldRenderFace(level_source, x, y, z, face);
    }
    int getColor(LevelSource *level_source, const int x, const int y, const int z) override {
        return GrassTile::VTable::base->getColor((GrassTile *) (Tile *) self, level_source, x, y, z);
    }
};
static Tile *get_fake_grass_side_tile() {
    static GrassSideTile out(0, 38, Material::dirt);
    return out.self;
}
static bool TileRenderer_tesselateBlockInWorld_injection(TileRenderer_tesselateBlockInWorld_t original, TileRenderer *self, Tile *tile, const int x, const int y, const int z) {
    const bool ret = original(self, tile, x, y, z);
    if (tile == Tile::grass && tile->getTexture3(self->level, x, y, z, 2) == 3) {
        original(self, get_fake_grass_side_tile(), x, y, z);
    }
    return ret;
}

// No Block Tinting
template <typename T>
static int32_t Tile_getColor_injection(MCPI_UNUSED const std::function<int(T *, LevelSource *, int, int, int)> &original, MCPI_UNUSED T *self, MCPI_UNUSED LevelSource *level_source, MCPI_UNUSED int x, MCPI_UNUSED int y, MCPI_UNUSED int z) {
    return 0xffffff;
}

// Init
void _init_textures_tinting() {
    // Change Grass Color
    if (feature_has("Add Biome Colors To Grass", server_disabled)) {
        overwrite_calls(Minecraft_init, Minecraft_init_injection);
        overwrite_calls(GrassTile_getColor, GrassTile_getColor_injection);
        overwrite_calls(TallGrass_getColor, TallGrass_getColor_injection);
        // Fancy Grass Side
        // This Requires Alpha Testing On The Opaque Render-Layer, Which Could Hurt Performance
        overwrite_calls(TileRenderer_tesselateBlockInWorld, TileRenderer_tesselateBlockInWorld_injection);
        overwrite_call_manual((void *) 0x4a038, (void *) media_glEnable);
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