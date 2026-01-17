#include <libreborn/patch.h>
#include <libreborn/util/util.h>

#include <symbols/IceTile.h>
#include <symbols/LevelSource.h>
#include <symbols/Minecraft.h>

#include <mods/misc/misc.h>

#include "../internal.h"
#include "patcher.h"

// Enable Translucent Rendering
static int IceTile_getRenderLayer_injection(MCPI_UNUSED IceTile *self) {
    return 2;
}
static bool IceTile_isSolidRender_injection(MCPI_UNUSED IceTile *self) {
    return false;
}
static bool IceTile_shouldRenderFace_injection(IceTile *self, LevelSource *level, const int x, const int y, const int z, const int face) {
    const int id = level->getTile(x, y, z);
    if (id == self->id) {
        return false;
    }
    return Tile_shouldRenderFace->get(false)((Tile *) self, level, x, y, z, face);
}

// Patch Texture
static void patch_texture(const Minecraft *minecraft) {
    // Color Conversion
    static const std::unordered_map<uchar, uchar> r_map = {
        {170, 119},
        {184, 142},
        {255, 255}
    };
    static const std::unordered_map<uchar, uchar> g_map = {
        {201, 169},
        {215, 191},
        {255, 255}
    };
    static const std::unordered_map<uchar, uchar> b_map = {
        {255, 255}
    };
    static const std::unordered_map<uchar, uchar> a_map = {
        {255, 159}
    };

    // Prepare
    Textures *textures = minecraft->textures;
    const AtlasPatcher patcher("terrain.png", textures);

    // Patch
    const int texture = Tile::ice->getTexture1(0);
    const bool success = patcher.patch(texture, texture, [](uchar &r, uchar &g, uchar &b, uchar &a) {
        // Patch A Pixel
        const std::vector<std::pair<uchar &, const std::unordered_map<uchar, uchar> &>> color_maps = {
            {r, r_map},
            {g, g_map},
            {b, b_map},
            {a, a_map}
        };
        for (const std::pair<uchar &, const std::unordered_map<uchar, uchar> &> &pair : color_maps) {
            // Patch A Channel
            uchar &i = pair.first;
            const std::unordered_map<uchar, uchar> &map = pair.second;
            if (map.contains(i)) {
                i = map.at(i);
            } else {
                // Unexpected Color
                // Do Not Patch The Texture
                return false;
            }
        }
        // Successfully Patched Pixel
        return true;
    });
    if (!success) {
        WARN("Unable To Patch Ice Texture");
    }
}
static void Minecraft_onGraphicsReset_injection(Minecraft_onGraphicsReset_t original, Minecraft *self) {
    // Call Original Method
    original(self);
    // Patch Texture
    patch_texture(self);
}

// Init
void _patch_ice_texture() {
    patch_vtable(IceTile_getRenderLayer, IceTile_getRenderLayer_injection);
    patch_vtable(IceTile_isSolidRender, IceTile_isSolidRender_injection);
    patch_vtable(IceTile_shouldRenderFace, IceTile_shouldRenderFace_injection);
    misc_run_on_init(patch_texture);
    overwrite_calls(Minecraft_onGraphicsReset, Minecraft_onGraphicsReset_injection);
}