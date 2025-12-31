#include <libreborn/patch.h>
#include <libreborn/util/util.h>

#include <symbols/IceTile.h>
#include <symbols/LevelSource.h>
#include <symbols/Minecraft.h>
#include <symbols/Textures.h>
#include <symbols/Texture.h>

#include <GLES/gl.h>

#include <mods/misc/misc.h>

#include "internal.h"

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
static int get_line_size(const int width, const int channels) {
    int line_size = width * channels;
    int alignment;
    media_glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    line_size = align_up(line_size, alignment);
    return line_size;
}
static void patch_texture(const Minecraft *minecraft) {
    // Constants
    constexpr int atlas_size_in_tiles = 16;
    constexpr int tile_size = 16;
    constexpr int atlas_size_in_pixels = atlas_size_in_tiles * tile_size;
    constexpr int rgb = 3;
    constexpr int rgba = rgb + 1;

    // Color Conversion
    const std::unordered_map<uchar, uchar> r_map = {
        {170, 119},
        {184, 142},
        {255, 255}
    };
    const std::unordered_map<uchar, uchar> g_map = {
        {201, 169},
        {215, 191},
        {255, 255}
    };
    const std::unordered_map<uchar, uchar> b_map = {
        {255, 255}
    };
    const std::unordered_map<uchar, uchar> a_map = {
        {255, 159}
    };

    // Get Texture Data
    Textures *textures = minecraft->textures;
    const uint texture_id = textures->loadAndBindTexture("terrain.png");
    const Texture *texture_data = textures->getTemporaryTextureData(texture_id);
    if (!texture_data) {
        return;
    }

    // Get Texture Information
    const int texture = Tile::ice->getTexture1(0);
    int src_x = texture % atlas_size_in_tiles;
    int src_y = texture / atlas_size_in_tiles;
    for (int *x : {&src_x, &src_y}) {
        *x *= tile_size;
    }
    const float x_scale = float(texture_data->width) / float(atlas_size_in_pixels);
    const float y_scale = float(texture_data->height) / float(atlas_size_in_pixels);
    src_x = int(float(src_x) * x_scale);
    src_y = int(float(src_y) * y_scale);
    const int width = int(float(tile_size) * x_scale);
    const int height = int(float(tile_size) * y_scale);
    const int src_channels = texture_data->has_alpha ? rgba : rgb;
    constexpr int dst_channels = rgba;
    const int src_line_size = get_line_size(texture_data->width, src_channels);
    const int dst_line_size = get_line_size(width, dst_channels);

    // Patch
    const uchar *old_texture = texture_data->data;
    uchar *new_texture = new uchar[dst_line_size * height];
    bool success = true;
    for (int y = 0; y < height && success; y++) {
        for (int x = 0; x < width && success; x++) {
            // Extract Color Channels
            const int src_position = ((src_y + y) * src_line_size) + ((src_x + x) * src_channels);
            const int dst_position = (y * dst_line_size) + (x * dst_channels);
            uchar r = old_texture[src_position];
            uchar g = old_texture[src_position + 1];
            uchar b = old_texture[src_position + 2];
            uchar a = old_texture[src_position + 3];
            // Map Color Channels
            const std::vector<std::pair<uchar &, const std::unordered_map<uchar, uchar> &>> color_maps = {
                {r, r_map},
                {g, g_map},
                {b, b_map},
                {a, a_map}
            };
            for (const std::pair<uchar &, const std::unordered_map<uchar, uchar> &> &pair : color_maps) {
                uchar &i = pair.first;
                const std::unordered_map<uchar, uchar> &map = pair.second;
                if (map.contains(i)) {
                    i = map.at(i);
                } else {
                    // Unexpected Color
                    // Do Not Patch The Texture
                    WARN("Unable To Patch Ice Texture");
                    success = false;
                    break;
                }
            }
            // Set New Color
            if (success) {
                new_texture[dst_position] = r;
                new_texture[dst_position + 1] = g;
                new_texture[dst_position + 2] = b;
                new_texture[dst_position + 3] = a;
            }
        }
    }
    if (success) {
        media_glTexSubImage2D(GL_TEXTURE_2D, 0, src_x, src_y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, new_texture);
    }
    delete[] new_texture;
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