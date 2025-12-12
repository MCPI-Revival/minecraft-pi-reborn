#include <libreborn/patch.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>

#include <symbols/Textures.h>

#include <media-layer/core.h>

#include <mods/misc/misc.h>
#include "internal.h"

// Loading Pending Skins
static void load_pending_skins(MCPI_UNUSED const Minecraft *minecraft) {
    media_apply_downloaded_textures();
}

// Skin Server
static std::string get_skin_server() {
    const char *custom_server = getenv(MCPI_SKIN_SERVER_ENV);
    if (custom_server != nullptr) {
        return custom_server;
    } else {
        return reborn_config.extra.skin_server;
    }
}

// Intercept Texture Creation
static int32_t Textures_assignTexture_injection(Textures_assignTexture_t original, Textures *textures, const std::string &name, const Texture &data) {
    // Call Original Method
    const int32_t id = original(textures, name, data);

    // Load Skin
    if (name.starts_with("$")) {
        const std::string url = get_skin_server() + '/' + name.substr(1) + ".png";
        media_download_into_texture(id, url.c_str());
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
