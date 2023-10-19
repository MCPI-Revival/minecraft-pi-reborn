#include <pthread.h>
#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <png.h>
#include <GLES/gl.h>

#include <mods/misc/misc.h>
#include <mods/textures/textures.h>
#include "skin-internal.h"

// Constants
#define SKIN_WIDTH 64
#define SKIN_HEIGHT 32

// Loading Pending Skins
struct pending_skin {
    int32_t texture_id;
    char *data;
};
static std::vector<pending_skin> &get_pending_skins() {
    static std::vector<pending_skin> pending_skins;
    return pending_skins;
}
static pthread_mutex_t pending_skins_lock = PTHREAD_MUTEX_INITIALIZER;
static void read_from_png(png_structp pngPtr, png_bytep data, png_size_t length) {
    char **src = (char **) png_get_io_ptr(pngPtr);
    memcpy(data, *src, length);
    *src += length;
}
static void load_pending_skins(__attribute__((unused)) unsigned char *minecraft) {
    // Lock
    pthread_mutex_lock(&pending_skins_lock);

    // Loop
    for (pending_skin &skin : get_pending_skins()) {
        // Init LibPNG
        png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!pngPtr) {
            continue;
        }
        png_infop infoPtr = png_create_info_struct(pngPtr);
        if (!infoPtr) {
            png_destroy_read_struct(&pngPtr, NULL, NULL);
            continue;
        }

        // Read PNG Info
        char *cursor = skin.data;
        png_set_read_fn(pngPtr, (png_voidp) &cursor, read_from_png);
        png_read_info(pngPtr, infoPtr);
        int width = png_get_image_width(pngPtr, infoPtr);
        int height = png_get_image_height(pngPtr, infoPtr);
        if (png_get_color_type(pngPtr, infoPtr) != PNG_COLOR_TYPE_RGBA) {
            continue;
        }
        if (width != SKIN_WIDTH || height != SKIN_HEIGHT) {
            continue;
        }
        int pixelSize = 4;

        // Read Image
        png_bytep *rowPtrs = new png_bytep[height];
        unsigned char *data = new unsigned char[pixelSize * width * height];
        int rowStrideBytes = pixelSize * width;
        for (int i = 0; i < height; i++) {
            rowPtrs[i] = (png_bytep) &data[i * rowStrideBytes];
        }
        png_read_image(pngPtr, rowPtrs);

        // Load Texture
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glBindTexture(GL_TEXTURE_2D, skin.texture_id);
        glTexSubImage2D_with_scaling(GL_TEXTURE_2D, 0, 0, 0, width, height, SKIN_WIDTH, SKIN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, last_texture);

        // Free
        delete[] data;
        png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp) 0);
        delete[] rowPtrs;
    }

    // Free
    for (pending_skin &skin : get_pending_skins()) {
        free(skin.data);
    }

    // Clear
    get_pending_skins().clear();

    // Unlock
    pthread_mutex_unlock(&pending_skins_lock);
}

// Skin Loader
struct loader_data {
    int32_t texture_id;
    std::string name;
};
static void *loader_thread(void *user_data) {
    // Loader Data
    loader_data *data = (loader_data *) user_data;

    // Download
    std::string url = std::string(MCPI_SKIN_SERVER) + '/' + data->name + ".png";
    int return_code;
    const char *command[] = {"wget", "-O", "-", url.c_str(), NULL};
    char *output = run_command(command, &return_code);

    // Check Success
    if (output != NULL && is_exit_status_success(return_code)) {
        // Success
        DEBUG("Downloaded Skin: %s", data->name.c_str());

        // Add To Pending Skins
        pending_skin skin;
        skin.texture_id = data->texture_id;
        skin.data = output;
        pthread_mutex_lock(&pending_skins_lock);
        get_pending_skins().push_back(skin);
        pthread_mutex_unlock(&pending_skins_lock);
    } else {
        // Failure
        WARN("Failed To Download Skin: %s", data->name.c_str());
        free(output);
    }

    // Free
    delete data;
    return NULL;
}

// Intercept Texture Creation
static int32_t Textures_assignTexture_injection(unsigned char *textures, std::string const& name, unsigned char *data) {
    // Call Original Method
    int32_t id = (*Textures_assignTexture)(textures, name, data);

    // Load Skin
    if (starts_with(name.c_str(), "$")) {
        loader_data *user_data = new loader_data;
        user_data->name = name.substr(1);
        DEBUG("Loading Skin: %s", user_data->name.c_str());
        user_data->texture_id = id;
        // Start Thread
        pthread_t thread;
        pthread_create(&thread, NULL, loader_thread, (void *) user_data);
    }

    // Return
    return id;
}

// Init
void _init_skin_loader() {
    // Intercept Texture Creation
    overwrite_calls((void *) Textures_assignTexture, (void *) Textures_assignTexture_injection);
    // Pending Skins
    misc_run_on_tick(load_pending_skins);
}
