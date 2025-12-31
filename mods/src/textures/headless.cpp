#include <libreborn/patch.h>
#include <symbols/AppPlatform_linux.h>

#include "internal.h"

// Disable Texture Loading
static Texture AppPlatform_linux_loadTexture_injection(MCPI_UNUSED AppPlatform_linux_loadTexture_t original, MCPI_UNUSED AppPlatform_linux *app_platform, MCPI_UNUSED const std::string &path, MCPI_UNUSED bool b) {
    Texture out = {};
    out.width = 0;
    out.height = 0;
    out.data = nullptr;
    out.data_size = 0;
    out.has_alpha = true;
    out.prevent_freeing_data = false;
    out.field6_0x14 = 0;
    out.field7_0x18 = -1;
    return out;
}

// Init
void _init_textures_headless() {
    // Disable Texture Loading
    overwrite_calls(AppPlatform_linux_loadTexture, AppPlatform_linux_loadTexture_injection);
}
