#include <pthread.h>

#include <libreborn/util/exec.h>
#include "stb_image.h"

#include <GLES/gl.h>
#include "../media.h"

// Apply Downloaded Textures
struct pending_texture {
    pthread_t thread = {};
    // Parameters
    GLuint texture_id = 0;
    std::string url;
    // Texture Data
    const std::vector<unsigned char> *data = nullptr;
    // State
    bool success = false;
    bool cancelled = false;
};
static std::vector<pending_texture *> pending_textures;
static pthread_mutex_t pending_textures_lock = PTHREAD_MUTEX_INITIALIZER;
void media_apply_downloaded_textures() {
    // Lock
    pthread_mutex_lock(&pending_textures_lock);

    // Loop
    std::vector<const pending_texture *> done;
    for (const pending_texture *texture : pending_textures) {
        // Check Status
        if (!texture->data) {
            // Not Finished Loading
            continue;
        }

        // Check Result
        if (!texture->cancelled && texture->success) {
            // PNG Was Loaded Successfully

            // Read PNG Information
            int width = 0, height = 0, channels = 0;
            stbi_uc *img = stbi_load_from_memory(texture->data->data(), int(texture->data->size()), &width, &height, &channels, STBI_rgb_alpha);

            // Load Texture
            GLint last_texture;
            media_glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
            media_glBindTexture(GL_TEXTURE_2D, texture->texture_id);
            media_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
            media_glBindTexture(GL_TEXTURE_2D, last_texture);

            // Free
            stbi_image_free(img);
        }

        // Mark As Done
        done.push_back(texture);
    }

    // Clear
    for (const pending_texture *texture : done) {
        pthread_join(texture->thread, nullptr);
        delete texture->data;
        delete texture;
        std::erase(pending_textures, texture);
    }

    // Unlock
    pthread_mutex_unlock(&pending_textures_lock);
}

// Start Download
static void *loader_thread(void *user_data) {
    // Loader Data
    pending_texture *data = (pending_texture *) user_data;
    const std::string &url = data->url;

    // Download
    int return_code;
    const char *command[] = {"wget", "-O", "-", url.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(command, &return_code);

    // Check Success
    const bool success = is_exit_status_success(return_code);
    if (success) {
        DEBUG("Downloaded Texture: %s", url.c_str());
    } else {
        WARN("Unable To Download Texture: %s", url.c_str());
    }

    // Mark As Downloaded
    pthread_mutex_lock(&pending_textures_lock);
    data->success = success;
    data->data = output;
    pthread_mutex_unlock(&pending_textures_lock);
    return nullptr;
}
void media_download_into_texture(const unsigned int texture, const char *url) {
    // Configure Thread
    DEBUG("Loading Texture: %s", url);
    pending_texture *data = new pending_texture;
    data->url = url;
    data->texture_id = texture;
    // Store Pending Texture
    pthread_mutex_lock(&pending_textures_lock);
    pending_textures.push_back(data);
    pthread_mutex_unlock(&pending_textures_lock);
    // Start Thread
    pthread_create(&data->thread, nullptr, loader_thread, data);
}

// Cancel Download
void _media_cancel_download(const unsigned int texture_id) {
    pthread_mutex_lock(&pending_textures_lock);
    for (pending_texture *texture : pending_textures) {
        if (texture->texture_id == texture_id) {
            DEBUG("Cancelled Texture Download: %s", texture->url.c_str());
            texture->cancelled = true;
        }
    }
    pthread_mutex_unlock(&pending_textures_lock);
}