#include <vector>
#include <cassert>
#include <cstdint>

#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/textures/textures.h>
#include <mods/atlas/atlas.h>
#include <mods/init/init.h>
#include "textures-internal.h"

#include "stb_image.h"

// Animated Water
static void Minecraft_tick_injection(const Minecraft *minecraft) {
    // Tick Dynamic Textures
    Textures *textures = minecraft->textures;
    if (textures != nullptr) {
        for (DynamicTexture *texture : textures->dynamic_textures) {
            texture->tick();
            texture->bindTexture(textures);
            for (int x = 0; x < texture->texture_size; x++) {
                for (int y = 0; y < texture->texture_size; y++) {
                    const Texture *data = textures->getTemporaryTextureData(textures->current_texture);
                    const int x_offset = 16 * ((texture->texture_index % 16) + x);
                    const int y_offset = 16 * ((texture->texture_index / 16) + y);
                    media_glTexSubImage2D_with_scaling(data, x_offset, y_offset, 16, 16, 256, 256, texture->pixels);
                }
            }
            atlas_update_tile(textures, texture->texture_index, texture->pixels);
        }
    }
}

// Scale Texture (Remember To Free)
#define PIXEL_SIZE 4
static int get_line_size(const int width) {
    int line_size = width * PIXEL_SIZE;
    {
        // Handle Alignment
        int alignment;
        media_glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        // Round
        line_size = ALIGN_UP(line_size, alignment);
    }
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
void media_glTexSubImage2D_with_scaling(const Texture *target, const GLint xoffset, const GLint yoffset, const GLsizei width, const GLsizei height, const GLsizei normal_texture_width, const GLsizei normal_texture_height, const void *pixels) {
    // Get Current Texture Size
    const GLsizei texture_width = target->width;
    const GLsizei texture_height = target->height;

    // Calculate Factor
    const float width_factor = ((float) texture_width) / ((float) normal_texture_width);
    const float height_factor = ((float) texture_height) / ((float) normal_texture_height);

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
static Texture AppPlatform_linux_loadTexture_injection(__attribute__((unused)) AppPlatform_linux_loadTexture_t original, __attribute__((unused)) AppPlatform_linux *app_platform, const std::string &path, const bool b) {
    Texture out;
    std::string real_path = path;
    if (b) {
        real_path = "data/images/" + real_path;
    }

    // Empty Texture
    out.width = 0;
    out.height = 0;
    out.data = nullptr;
    out.field3_0xc = 0;
    out.field4_0x10 = true;
    out.field5_0x11 = false;
    out.field6_0x14 = 0;
    out.field7_0x18 = -1;

    // Read Image
    int width = 0, height = 0, channels = 0;
    stbi_uc *img = stbi_load(real_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!img)
    {
        // Failed To Parse Image
        WARN("Unable To Load Texture: %s", real_path.c_str());
        return out;
    }

    // Copy Image
    unsigned char *img2 = new unsigned char[width * height * channels];
    memcpy(img2, img, width * height * channels);
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
        // Disable Animated Water If Set
        if (!animated_water) {
            unsigned char disable_water_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
            patch((void *) 0x17094, disable_water_patch);
            patch((void *) 0x170b4, disable_water_patch);
        }
        // Animated Lava
        _init_textures_lava(animated_water, animated_lava, animated_fire);
    }

    // Load Textures
    overwrite_calls(AppPlatform_linux_loadTexture, AppPlatform_linux_loadTexture_injection);

    // Stop Reloading Textures On Resize
    unsigned char texture_reset_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x126b4, texture_reset_patch);
}
