#include <pthread.h>
#include <vector>

#include <libreborn/patch.h>
#include <libreborn/env.h>
#include <libreborn/exec.h>
#include <libreborn/config.h>

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
    const std::vector<unsigned char> *data;
};
static std::vector<pending_skin> &get_pending_skins() {
    static std::vector<pending_skin> pending_skins;
    return pending_skins;
}
static pthread_mutex_t pending_skins_lock = PTHREAD_MUTEX_INITIALIZER;
static void load_pending_skins(const Minecraft *minecraft) {
    // Lock
    pthread_mutex_lock(&pending_skins_lock);

    // Loop
    for (const pending_skin &skin : get_pending_skins()) {
        // Read PNG Info
        int width = 0, height = 0, channels = 0;
        stbi_uc *img = stbi_load_from_memory(skin.data->data(), skin.data->size(), &width, &height, &channels, STBI_rgb_alpha);
        if (width != SKIN_WIDTH || height != SKIN_HEIGHT) {
            continue;
        }

        // Load Texture
        GLint last_texture;
        media_glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        media_glBindTexture(GL_TEXTURE_2D, skin.texture_id);
        const Texture *texture = minecraft->textures->getTemporaryTextureData(skin.texture_id);
        media_glTexSubImage2D_with_scaling(texture, 0, 0, width, height, SKIN_WIDTH, SKIN_HEIGHT, img);
        media_glBindTexture(GL_TEXTURE_2D, last_texture);

        // Free
        stbi_image_free(img);
    }

    // Free
    for (const pending_skin &skin : get_pending_skins()) {
        delete skin.data;
    }

    // Clear
    get_pending_skins().clear();

    // Unlock
    pthread_mutex_unlock(&pending_skins_lock);
}

// Skin Server
static std::string get_skin_server() {
    const char *custom_server = getenv(MCPI_SKIN_SERVER_ENV);
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
    const loader_data *data = (loader_data *) user_data;

    // Download
    const std::string url = get_skin_server() + '/' + data->name + ".png";
    int return_code;
    const char *command[] = {"wget", "-O", "-", url.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(command, &return_code);

    // Check Success
    if (is_exit_status_success(return_code)) {
        // Success
        DEBUG("Downloaded Skin: %s", data->name.c_str());

        // Add To Pending Skins
        pending_skin skin = {};
        skin.texture_id = data->texture_id;
        skin.data = output;
        pthread_mutex_lock(&pending_skins_lock);
        get_pending_skins().push_back(skin);
        pthread_mutex_unlock(&pending_skins_lock);
    } else {
        // Failure
        WARN("Failed To Download Skin: %s", data->name.c_str());
    }

    // Free
    delete data;
    return nullptr;
}

// Intercept Texture Creation
static int32_t Textures_assignTexture_injection(Textures_assignTexture_t original, Textures *textures, const std::string &name, const Texture &data) {
    // Call Original Method
    const int32_t id = original(textures, name, data);

    // Load Skin
    if (name.starts_with("$")) {
        loader_data *user_data = new loader_data;
        user_data->name = name.substr(1);
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
