#include <pthread.h>
#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <GLES/gl.h>

#include <mods/misc/misc.h>
#include <mods/textures/textures.h>
#include "skin-internal.h"

#include "stb_image.h"

// Constants
#define SKIN_WIDTH 64
#define SKIN_HEIGHT 32

// Loading Pending Skins
struct pending_skin {
    int32_t texture_id;
    char *data;
    int size;
};
static std::vector<pending_skin> &get_pending_skins() {
    static std::vector<pending_skin> pending_skins;
    return pending_skins;
}
static pthread_mutex_t pending_skins_lock = PTHREAD_MUTEX_INITIALIZER;
static void load_pending_skins(__attribute__((unused)) Minecraft *minecraft) {
    // Lock
    pthread_mutex_lock(&pending_skins_lock);

    // Loop
    for (pending_skin &skin : get_pending_skins()) {
        // Read PNG Info
        int width = 0, height = 0, channels = 0;
        stbi_uc *img = stbi_load_from_memory((unsigned char *) skin.data, skin.size, &width, &height, &channels, STBI_rgb_alpha);
        if (width != SKIN_WIDTH || height != SKIN_HEIGHT) {
            continue;
        }

        // Load Texture
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glBindTexture(GL_TEXTURE_2D, skin.texture_id);
        glTexSubImage2D_with_scaling(GL_TEXTURE_2D, 0, 0, 0, width, height, SKIN_WIDTH, SKIN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, img);
        glBindTexture(GL_TEXTURE_2D, last_texture);

        // Free
        stbi_image_free(img);
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

// Skin Server
static std::string get_skin_server() {
    const char *custom_server = getenv("MCPI_SKIN_SERVER");
    if (custom_server != nullptr) {
        return custom_server;
    } else {
        return MCPI_SKIN_SERVER;
    }
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
    std::string url = get_skin_server() + '/' + data->name + ".png";
    int return_code;
    const char *command[] = {"wget", "-O", "-", url.c_str(), nullptr};
    size_t output_size = 0;
    char *output = run_command(command, &return_code, &output_size);

    // Check Success
    if (output != nullptr && is_exit_status_success(return_code)) {
        // Success
        DEBUG("Downloaded Skin: %s", data->name.c_str());

        // Add To Pending Skins
        pending_skin skin;
        skin.texture_id = data->texture_id;
        skin.data = output;
        skin.size = (int) output_size;
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
    return nullptr;
}

// Intercept Texture Creation
static int32_t Textures_assignTexture_injection(Textures_assignTexture_t original, Textures *textures, std::string *name, unsigned char *data) {
    // Call Original Method
    int32_t id = original(textures, name, data);

    // Load Skin
    if (starts_with(name->c_str(), "$")) {
        loader_data *user_data = new loader_data;
        user_data->name = name->substr(1);
        DEBUG("Loading Skin: %s", user_data->name.c_str());
        user_data->texture_id = id;
        // Start Thread
        pthread_t thread;
        pthread_create(&thread, nullptr, loader_thread, (void *) user_data);
    }

    // Return
    return id;
}

// Init
void _init_skin_loader() {
    // Intercept Texture Creation
    overwrite_calls(Textures_assignTexture, Textures_assignTexture_injection);
    // Pending Skins
    misc_run_on_tick(load_pending_skins);
    // Log
    DEBUG("Skin Server: %s", get_skin_server().c_str());
}

