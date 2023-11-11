#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/init/init.h>

// Disable Texture Loading
static Texture AppPlatform_linux_loadTexture_injection(__attribute__((unused)) unsigned char *app_platform, __attribute__((unused)) std::string const& path, __attribute__((unused)) bool b) {
    return Texture();
}

// Init
void init_textures() {
    // Disable Texture Loading
    overwrite((void *) AppPlatform_linux_loadTexture, (void *) AppPlatform_linux_loadTexture_injection);
}
