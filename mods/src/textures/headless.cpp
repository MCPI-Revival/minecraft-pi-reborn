#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/init/init.h>

// Disable Texture Loading
static Texture AppPlatform_linux_loadTexture_injection(__attribute__((unused)) AppPlatform_linux *app_platform, __attribute__((unused)) std::string *path, __attribute__((unused)) bool b) {
    Texture out;
    out.width = 0;
    out.height = 0;
    out.data = NULL;
    out.field3_0xc = 0;
    out.field4_0x10 = true;
    out.field5_0x11 = false;
    out.field6_0x14 = 0;
    out.field7_0x18 = -1;
    return out;
}

// Init
void init_textures() {
    // Disable Texture Loading
    overwrite((void *) AppPlatform_linux_loadTexture_non_virtual, (void *) AppPlatform_linux_loadTexture_injection);
}
