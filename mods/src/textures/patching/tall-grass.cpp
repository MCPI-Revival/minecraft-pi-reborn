#include <limits>

#include "patcher.h"
#include "../internal.h"

#include <unordered_map>
#include <ranges>

#include <libreborn/patch.h>

#include <symbols/Minecraft.h>
#include <symbols/Tile.h>
#include <symbols/TallGrass.h>
#include <symbols/AuxDataTileItem.h>

#include <mods/misc/misc.h>

// Output Location In Atlas
static std::unordered_map<int, int> aux_to_texture = {
    {1, 202},
    {3, 203}
};

// Patch A Texture
static void multiply_channel(uchar &a, const uchar b) {
    constexpr uchar max = std::numeric_limits<uchar>::max();
    const float c = float(a) / max;
    const float d = float(b) / max;
    a = uchar(c * d * max);
}
static void patch_texture(const Minecraft *minecraft, const int auxiliary) {
    // Prepare
    Textures *textures = minecraft->textures;
    const AtlasPatcher patcher("terrain.png", textures);

    // Get Texture Location
    TallGrass *tile = (TallGrass *) Tile::tallgrass;
    const int input = tile->getTexture2(0, auxiliary);
    const int output = aux_to_texture.at(auxiliary);

    // Patch
    (void) patcher.patch(input, output, [&tile, &auxiliary](uchar &r, uchar &g, uchar &b, MCPI_UNUSED uchar &a) {
        const int color = tile->getColor_two(auxiliary);
        constexpr uchar mask = 0xff;
        const uchar color_r = (color >> 16) & mask;
        const uchar color_g = (color >> 8) & mask;
        const uchar color_b = color & mask;
        multiply_channel(r, color_r);
        multiply_channel(g, color_g);
        multiply_channel(b, color_b);
        return true;
    });
}

// Patch Textures
static void patch_textures(const Minecraft *minecraft) {
    // Patch All Variants
    for (const int auxiliary : aux_to_texture | std::views::keys) {
        patch_texture(minecraft, auxiliary);
    }
}
static void Minecraft_onGraphicsReset_injection(Minecraft_onGraphicsReset_t original, Minecraft *self) {
    // Handle Graphics Reset
    original(self);
    patch_textures(self);
}

// Use Patched Texture
static int AuxDataTileItem_getIcon_injection(AuxDataTileItem_getIcon_t original, AuxDataTileItem *self, const int auxiliary) {
    if (self->icon_tile->id == Tile::tallgrass->id && aux_to_texture.contains(auxiliary)) {
        return aux_to_texture.at(auxiliary);
    } else {
        // Call Original Method
        return original(self, auxiliary);
    }
}

// Init
void _patch_tall_grass_textures() {
    misc_run_on_init(patch_textures);
    overwrite_calls(Minecraft_onGraphicsReset, Minecraft_onGraphicsReset_injection);
    overwrite_calls(AuxDataTileItem_getIcon, AuxDataTileItem_getIcon_injection);
}