#include <vector>
#include <cassert>
#include <cstdint>

#include <GLES/gl.h>

#include <libreborn/patch.h>
#include <libreborn/util/util.h>
#include <libreborn/config.h>
#include <libreborn/env/env.h>

#include <symbols/Textures.h>
#include <symbols/Texture.h>
#include <symbols/DynamicTexture.h>
#include <symbols/Minecraft.h>
#include <symbols/AppPlatform_linux.h>

#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/textures/textures.h>
#include <mods/atlas/atlas.h>
#include <mods/init/init.h>
#include "internal.h"

#include "stb_image.h"

// Animated Water
static void tick_textures(Textures *textures) {
    constexpr int atlas_size = 16;
    constexpr int atlas_tile_size = 16;
    for (DynamicTexture *texture : textures->dynamic_textures) {
        texture->tick();
        texture->bindTexture(textures);
        for (int x = 0; x < texture->texture_size; x++) {
            for (int y = 0; y < texture->texture_size; y++) {
                const Texture *data = textures->getTemporaryTextureData(textures->current_texture);
                const int x_offset = atlas_tile_size * ((texture->texture_index % atlas_size) + x);
                const int y_offset = atlas_tile_size * ((texture->texture_index / atlas_size) + y);
                media_glTexSubImage2D_with_scaling(data, x_offset, y_offset, atlas_tile_size, atlas_tile_size, atlas_size * atlas_tile_size, atlas_size * atlas_tile_size, texture->pixels);
            }
        }
        if (textures->current_texture == textures->loadTexture("terrain.png", true)) {
            atlas_update_tile(textures, texture->texture_index, texture->pixels);
        }
    }
}
static void Minecraft_tick_injection(const Minecraft *minecraft) {
    // Tick Dynamic Textures
    Textures *textures = minecraft->textures;
    if (textures != nullptr) {
        int count = 1;
        if (is_env_set(MCPI_PROMOTIONAL_ENV)) {
            // Freeze Animated Textures In Promotional Mode
            static bool has_run = false;
            if (!has_run) {
                count = 40;
                has_run = true;
            } else {
                count = 0;
            }
        }
        for (int i = 0; i < count; i++) {
            tick_textures(textures);
        }
    }
}

// Scale Texture (Remember To Free)
#define PIXEL_SIZE 4
static int get_line_size(const int width) {
    int line_size = width * PIXEL_SIZE;
    // Handle Alignment
    static int alignment;
    static bool loaded = false;
    if (!loaded) {
        media_glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        loaded = true;
    }
    // Round
    line_size = align_up(line_size, alignment);
    return line_size;
}
static unsigned char *scale_texture(const unsigned char *src, const GLsizei old_width, const GLsizei old_height, const GLsizei new_width, const GLsizei new_height) {
    const int old_line_size = get_line_size(old_width);
    const int new_line_size = get_line_size(new_width);
    // Allocate
    unsigned char *dst = new unsigned char[new_height * new_line_size];
    // Scale
    for (int new_x = 0; new_x < new_width; new_x++) {
        for (int new_y = 0; new_y < new_height; new_y++) {
            const int old_x = (int) (((float) new_x / (float) new_width) * (float) old_width);
            const int old_y = (int) (((float) new_y / (float) new_height) * (float) old_height);
            // Find Position
            const int new_position = (new_y * new_line_size) + (new_x * PIXEL_SIZE);
            const int old_position = (old_y * old_line_size) + (old_x * PIXEL_SIZE);
            // Copy
            static_assert(sizeof (int32_t) == PIXEL_SIZE, "Pixel Size Doesn't Match 32-Bit Integer Size");
            *(int32_t *) &dst[new_position] = *(int32_t *) &src[old_position];
        }
    }
    // Return
    return dst;
}

// Scale Animated Textures
void media_glTexSubImage2D_with_scaling(const Texture *target, const GLint xoffset, const GLint yoffset, const GLsizei width, const GLsizei height, const GLfloat normal_texture_width, const GLfloat normal_texture_height, const void *pixels) {
    // Get Current Texture Size
    const GLsizei texture_width = target->width;
    const GLsizei texture_height = target->height;

    // Calculate Factor
    const float width_factor = float(texture_width) / normal_texture_width;
    const float height_factor = float(texture_height) / normal_texture_height;

    // Only Scale If Needed
    if (width_factor == 1.0f && height_factor == 1.0f) {
        // No Scaling
        media_glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    } else {
        // Scale
        const GLsizei new_width = GLsizei(float(width) * width_factor);
        const GLsizei new_height = GLsizei(float(height) * height_factor);
        const unsigned char *new_pixels = scale_texture((const unsigned char *) pixels, width, height, new_width, new_height);

        // Call Original Method
        const GLint new_xoffset = GLint(float(xoffset) * width_factor);
        const GLint new_yoffset = GLint(float(yoffset) * height_factor);
        media_glTexSubImage2D(GL_TEXTURE_2D, 0, new_xoffset, new_yoffset, new_width, new_height, GL_RGBA, GL_UNSIGNED_BYTE, new_pixels);

        // Free
        delete[] new_pixels;
    }
}

// Load Textures
static Texture AppPlatform_linux_loadTexture_injection(MCPI_UNUSED AppPlatform_linux_loadTexture_t original, MCPI_UNUSED AppPlatform_linux *app_platform, const std::string &path, const bool b) {
    Texture out = {};
    std::string real_path = path;
    if (b) {
        real_path = "data/images/" + real_path;
    }

    // Empty Texture
    out.width = 0;
    out.height = 0;
    out.data = nullptr;
    out.data_size = 0;
    out.has_alpha = true;
    out.prevent_freeing_data = false;
    out.field6_0x14 = 0;
    out.field7_0x18 = -1;

    // Read Image
    int width = 0, height = 0, channels = 0;
    constexpr int desired_channels = STBI_rgb_alpha;
    stbi_uc *img = stbi_load(real_path.c_str(), &width, &height, &channels, desired_channels);
    if (!img) {
        // Failed To Parse Image
        WARN("Unable To Load Texture: %s", real_path.c_str());
        return out;
    }

    // Get Line Size
    const int line_width = width * desired_channels;
    const int size = height * line_width;
    out.data_size = size;
    // Check Alignment
    int alignment;
    media_glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    if ((line_width % alignment) != 0) {
        ERR("Invalid Texture Width");
    }
    // Copy Image
    unsigned char *img2 = new unsigned char[size];
    memcpy(img2, img, size);
    stbi_image_free(img);

    // Create Texture
    out.width = width;
    out.height = height;
    out.data = img2;

    // Return
    return out;
}

// Init
void init_textures() {
    // Handle Headless Mode
    if (reborn_is_headless()) {
        _init_textures_headless();
        return;
    }

    // Tick Dynamic Textures (Animated Water)
    const bool animated_water = feature_has("Animated Water", server_disabled);
    const bool animated_lava = feature_has("Animated Lava", server_disabled);
    const bool animated_fire = feature_has("Animated Fire", server_disabled);
    if (animated_water || animated_lava || animated_fire) {
        // Tick Dynamic Textures
        misc_run_on_tick(Minecraft_tick_injection);
        // Animated Lava
        _init_textures_lava(animated_water, animated_lava, animated_fire);
    }

    // Load Textures
    overwrite_calls(AppPlatform_linux_loadTexture, AppPlatform_linux_loadTexture_injection);

    // Stop Reloading Textures On Resize
    if (feature_has("Fix Reloading Textures On Resize", server_disabled)) {
        unsigned char texture_reset_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x126b4, texture_reset_patch);
    }

    // Block Tinting
    _init_textures_tinting();

    // Ice
    if (feature_has("Translucent Ice", server_disabled)) {
        _patch_ice_texture();
    }
}
