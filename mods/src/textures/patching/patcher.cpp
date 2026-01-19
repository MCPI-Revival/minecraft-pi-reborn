#include "patcher.h"

#include <GLES/gl.h>
#include <libreborn/util/util.h>

#include <symbols/Textures.h>
#include <symbols/Texture.h>

#include <mods/common.h>

// Constructor
AtlasPatcher::AtlasPatcher(const std::string &atlas_, Textures *textures_):
    atlas(atlas_),
    textures(textures_) {}

// Atlas Information
static constexpr int atlas_size_in_tiles = AtlasSize::TILE_COUNT;
static constexpr int tile_size = AtlasSize::TILE_SIZE;
static constexpr int atlas_size_in_pixels = AtlasSize::SIZE;
static void get_position(const int texture, int &out_x, int &out_y) {
    out_x = texture % atlas_size_in_tiles;
    out_y = texture / atlas_size_in_tiles;
    for (int *x : {&out_x, &out_y}) {
        *x *= tile_size;
    }
}

// Get Line Size
static int get_alignment() {
    static int alignment;
    static bool loaded = false;
    if (!loaded) {
        media_glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        loaded = true;
    }
    return alignment;
}
static int get_line_size(const int width, const int channels) {
    int line_size = width * channels;
    const int alignment = get_alignment();
    line_size = align_up(line_size, alignment);
    return line_size;
}

// Patch
bool AtlasPatcher::patch(const int input, const int output, const Callback &callback) const {
    // Constants
    constexpr int rgb = 3;
    constexpr int rgba = rgb + 1;
    constexpr uchar fallback_value = 255;

    // Get Texture Data
    const uint texture_id = textures->loadAndBindTexture(atlas);
    const Texture *texture_data = textures->getTemporaryTextureData(texture_id);
    if (!texture_data) {
        return false;
    }

    // Get Intput/Output Positions
    int src_x;
    int src_y;
    get_position(input, src_x, src_y);
    int dst_x;
    int dst_y;
    get_position(output, dst_x, dst_y);

    // Scale Positions
    const float x_scale = float(texture_data->width) / float(atlas_size_in_pixels);
    const float y_scale = float(texture_data->height) / float(atlas_size_in_pixels);
    for (int *x : {&src_x, &dst_x}) {
        *x = int(float(*x) * x_scale);
    }
    for (int *y : {&src_y, &dst_y}) {
        *y = int(float(*y) * y_scale);
    }

    // Get Texture Information
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
            // Get Pixels
            const int src_position = ((src_y + y) * src_line_size) + ((src_x + x) * src_channels);
            const int dst_position = (y * dst_line_size) + (x * dst_channels);

            // Extract Color Channels
            const uchar *src = old_texture + src_position;
            uchar *dst = new_texture + dst_position;
            for (int channel = 0; channel < dst_channels; channel++) {
                dst[channel] = channel < src_channels ? src[channel] : fallback_value;
            }

            // Patch Pixel
            success = callback(dst[0], dst[1], dst[2], dst[3]);
        }
    }

    // Apply Patch
    if (success) {
        media_glTexSubImage2D(GL_TEXTURE_2D, 0, dst_x, dst_y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, new_texture);
    }
    delete[] new_texture;
    textures->current_texture = uint(-1);
    return success;
}
