#include <string>
#include <fstream>
#include <streambuf>

#include <cstring>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include "misc-internal.h"
#include <mods/misc/misc.h>

// Read Asset File
static AppPlatform_readAssetFile_return_value AppPlatform_readAssetFile_injection(__attribute__((unused)) AppPlatform *app_platform, std::string const& path) {
    // Open File
    std::ifstream stream("data/" + path, std::ios_base::binary | std::ios_base::ate);
    if (!stream) {
        // Does Not Exist
        AppPlatform_readAssetFile_return_value ret;
        ret.length = -1;
        ret.data = NULL;
        return ret;
    }
    // Read File
    long len = stream.tellg();
    char *buf = new char[len];
    stream.seekg(0, stream.beg);
    stream.read(buf, len);
    stream.close();
    // Return String
    AppPlatform_readAssetFile_return_value ret;
    ret.length = len;
    ret.data = strdup(buf);
    return ret;
}

// Add Missing Buttons To Pause Menu
static void PauseScreen_init_injection(PauseScreen *screen) {
    // Call Original Method
    PauseScreen_init_non_virtual(screen);

    // Check If Server
    Minecraft *minecraft = screen->minecraft;
    RakNetInstance *rak_net_instance = minecraft->rak_net_instance;
    if (rak_net_instance != NULL) {
        if (rak_net_instance->vtable->isServer(rak_net_instance)) {
            // Add Button
            std::vector<Button *> *rendered_buttons = &screen->rendered_buttons;
            std::vector<Button *> *selectable_buttons = &screen->selectable_buttons;
            Button *button = screen->server_visibility_button;
            rendered_buttons->push_back(button);
            selectable_buttons->push_back(button);

            // Update Button Text
            PauseScreen_updateServerVisibilityText(screen);
        }
    }
}

// Init
void _init_misc_cpp() {
    // Implement AppPlatform::readAssetFile So Translations Work
    if (feature_has("Load Language Files", server_enabled)) {
        overwrite((void *) *AppPlatform_readAssetFile_vtable_addr, (void *) AppPlatform_readAssetFile_injection);
    }

    // Fix Pause Menu
    if (feature_has("Fix Pause Menu", server_disabled)) {
        // Add Missing Buttons To Pause Menu
        patch_address(PauseScreen_init_vtable_addr, (void *) PauseScreen_init_injection);
    }
}
