#include <vector>
#include <assert.h>
#include <cstdint>

#include <GLES/gl.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/textures/textures.h>
#include <mods/init/init.h>
#include "textures-internal.h"

#include "stb_image.h"

// Animated Water
static void Minecraft_tick_injection(Minecraft *minecraft) {
    // Tick Dynamic Textures
    Textures *textures = minecraft->textures;
    if (textures != nullptr) {
        textures->tick(true);
    }
}

// Store Texture Sizes
struct texture_data {
    GLint id;
    GLsizei width;
    GLsizei height;
};
static std::vector<texture_data> &get_texture_data() {
    static std::vector<texture_data> data;
    return data;
}
HOOK(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)) {
    // Store
    texture_data data;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &data.id);
    data.width = width;
    data.height = height;
    get_texture_data().push_back(data);

    // Call Original Method
    ensure_glTexImage2D();
    real_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}
HOOK(glDeleteTextures, void, (GLsizei n, const GLuint *textures)) {
    // Remove Old Data
    for (int i = 0; i < n; i++) {
        GLint id = textures[i];
        std::vector<texture_data>::iterator it = get_texture_data().begin();
        while (it != get_texture_data().end()) {
            texture_data data = *it;
            if (data.id == id) {
                it = get_texture_data().erase(it);
            } else {
                ++it;
            }
        }
    }

    // Call Original Method
    ensure_glDeleteTextures();
    real_glDeleteTextures(n, textures);
}
static void get_texture_size(GLint id, GLsizei *width, GLsizei *height) {
    // Iterate
    std::vector<texture_data>::iterator it = get_texture_data().begin();
    while (it != get_texture_data().end()) {
        texture_data data = *it;
        if (data.id == id) {
            // Found
            *width = data.width;
            *height = data.height;
            return;
        }
        ++it;
    }
    // Not Found
    *width = 0;
    *height = 0;
}

// Scale Texture (Remember To Free)
#define PIXEL_SIZE 4
static int get_line_size(int width) {
    int line_size = width * PIXEL_SIZE;
    {
        // Handle Alignment
        int alignment;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        // Round
        int diff = line_size % alignment;
        if (diff > 0) {
            line_size = line_size + (alignment - diff);
        }
    }
    return line_size;
}
static void *scale_texture(const unsigned char *src, GLsizei old_width, GLsizei old_height, GLsizei new_width, GLsizei new_height) {
    int old_line_size = get_line_size(old_width);
    int new_line_size = get_line_size(new_width);

    // Allocate
    unsigned char *dst = (unsigned char *) malloc(new_height * new_line_size);
    ALLOC_CHECK(dst);

    // Scale
    for (int new_x = 0; new_x < new_width; new_x++) {
        int old_x = (int) (((float) new_x / (float) new_width) * (float) old_width);
        for (int new_y = 0; new_y < new_height; new_y++) {
            int old_y = (int) (((float) new_y / (float) new_height) * (float) old_height);

            // Find Position
            int new_position = (new_y * new_line_size) + (new_x * PIXEL_SIZE);
            int old_position = (old_y * old_line_size) + (old_x * PIXEL_SIZE);

            // Copy
            static_assert(sizeof (int32_t) == PIXEL_SIZE, "Pixel Size Doesn't Match 32-Bit Integer Size");
            *(int32_t *) &dst[new_position] = *(int32_t *) &src[old_position];
        }
    }

    // Return
    return dst;
}

// Scale Animated Textures
void glTexSubImage2D_with_scaling(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLsizei normal_texture_width, GLsizei normal_texture_height, GLenum format, GLenum type, const void *pixels) {
    // Get Current Texture Size
    GLint current_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);
    GLsizei texture_width;
    GLsizei texture_height;
    get_texture_size(current_texture, &texture_width, &texture_height);

    // Calculate Factor
    float width_factor = ((float) texture_width) / ((float) normal_texture_width);
    float height_factor = ((float) texture_height) / ((float) normal_texture_height);

    // Only Scale If Needed
    if (width_factor == 1.0f && height_factor == 1.0f) {
        // No Scaling
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    } else {
        // Check
        if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
            // Pixels Must Be 4 Bytes
            ERR("Unsupported Texture Format For Scaling");
        }

        // Scale
        GLsizei new_width = width * width_factor;
        GLsizei new_height = height * height_factor;
        void *new_pixels = scale_texture((const unsigned char *) pixels, width, height, new_width, new_height);

        // Call Original Method
        GLint new_xoffset = xoffset * width_factor;
        GLint new_yoffset = yoffset * height_factor;
        glTexSubImage2D(target, level, new_xoffset, new_yoffset, new_width, new_height, format, type, new_pixels);

        // Free
        free(new_pixels);
    }
}
static void Textures_tick_glTexSubImage2D_injection(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
    glTexSubImage2D_with_scaling(target, level, xoffset, yoffset, width, height, 256, 256, format, type, pixels);
}

// Load Textures
static Texture AppPlatform_linux_loadTexture_injection(__attribute__((unused)) AppPlatform_linux_loadTexture_t original, __attribute__((unused)) AppPlatform_linux *app_platform, std::string *path, bool b) {
    Texture out;
    std::string real_path = *path;
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
    // Tick Dynamic Textures (Animated Water)
    bool animated_water = feature_has("Animated Water", server_disabled);
    bool animated_lava = feature_has("Animated Lava", server_disabled);
    bool animated_fire = feature_has("Animated Fire", server_disabled);
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

    // Scale Animated Textures
    overwrite_call((void *) 0x53274, (void *) Textures_tick_glTexSubImage2D_injection);

    // Load Textures
    overwrite_calls(AppPlatform_linux_loadTexture, AppPlatform_linux_loadTexture_injection);
}
