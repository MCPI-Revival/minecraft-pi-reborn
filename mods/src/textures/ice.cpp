#include <libreborn/patch.h>
#include <libreborn/util/util.h>

#include <symbols/minecraft.h>
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
    return Tile_shouldRenderFace->get(false)((Tile *) self, level, x, y, z, 1 - face);
}

// Patch Texture
static int get_line_size(const int width, const int channels) {
    int line_size = width * channels;
    int alignment;
    media_glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    line_size = align_up(line_size, alignment);
    return line_size;
}
static void patch_texture(Minecraft *minecraft) {
    // Constants
    constexpr int atlas_size_in_tiles = 16;
    constexpr int tile_size = 16;
    constexpr int atlas_size_in_pixels = atlas_size_in_tiles * tile_size;
    constexpr int rgb = 3;
    constexpr int rgba = rgb + 1;

    // Color Conversion
    constexpr uchar desired_alpha = 159;
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
    const int src_channels = texture_data->field4_0x10 ? rgba : rgb;
    constexpr int dst_channels = rgba;
    const int src_line_size = get_line_size(texture_data->width, src_channels);
    const int dst_line_size = get_line_size(width, dst_channels);

    // Patch
    const uchar *old_texture = texture_data->data;
    uchar *new_texture = new uchar[dst_line_size * height];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const int src_position = ((src_y + y) * src_line_size) + ((src_x + x) * src_channels);
            const int dst_position = (y * dst_line_size) + (x * dst_channels);
            new_texture[dst_position] = r_map.at(old_texture[src_position]);
            new_texture[dst_position + 1] = g_map.at(old_texture[src_position + 1]);
            new_texture[dst_position + 2] = old_texture[src_position + 2];
            new_texture[dst_position + 3] = desired_alpha;
        }
    }
    media_glTexSubImage2D(GL_TEXTURE_2D, 0, src_x, src_y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, new_texture);
    delete[] new_texture;
}

// Init
void _patch_ice_texture() {
    patch_vtable(IceTile_getRenderLayer, IceTile_getRenderLayer_injection);
    patch_vtable(IceTile_isSolidRender, IceTile_isSolidRender_injection);
    patch_vtable(IceTile_shouldRenderFace, IceTile_shouldRenderFace_injection);
    misc_run_on_init(patch_texture);
}