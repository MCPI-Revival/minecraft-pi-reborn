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

// Animated Water
static void Minecraft_tick_injection(unsigned char *minecraft) {
    // Tick Dynamic Textures
    unsigned char *textures = *(unsigned char **) (minecraft + Minecraft_textures_property_offset);
    if (textures != NULL) {
        (*Textures_tick)(textures, true);
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
    (*real_glTexImage2D)(target, level, internalformat, width, height, border, format, type, pixels);
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
    (*real_glDeleteTextures)(n, textures);
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

// Init
void init_textures() {
    // Tick Dynamic Textures (Animated Water)
    if (feature_has("Animated Water", server_disabled)) {
        misc_run_on_tick(Minecraft_tick_injection);
    }

    // Scale Animated Textures
    overwrite_call((void *) 0x53274, (void *) Textures_tick_glTexSubImage2D_injection);
}
