#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

#include <cstring>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../feature/feature.h"
#include "misc.h"

// Read Asset File
static AppPlatform_readAssetFile_return_value AppPlatform_readAssetFile_injection(__attribute__((unused)) unsigned char *app_platform, std::string const& path) {
    // Read File
    std::string full_path("./data/");
    full_path.append(path);
    std::ifstream stream(full_path);
    std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    // Return String
    AppPlatform_readAssetFile_return_value ret;
    ret.length = str.length();
    ret.data = strdup(str.c_str());
    return ret;
}

// Run Functions On Input Tick
static std::vector<misc_update_function_t> &get_misc_update_functions() {
    static std::vector<misc_update_function_t> functions;
    return functions;
}
void misc_run_on_update(misc_update_function_t function) {
    get_misc_update_functions().push_back(function);
}

// Handle Custom Update Behavior
static void Minecraft_update_injection(unsigned char *minecraft) {
    // Call Original Method
    (*Minecraft_update)(minecraft);

    // Run Input Tick Functions
    for (misc_update_function_t function : get_misc_update_functions()) {
        (*function)(minecraft);
    }
}

// Add Missing Buttons To Pause Menu
static void PauseScreen_init_injection(unsigned char *screen) {
    // Call Original Method
    (*PauseScreen_init)(screen);

    // Check If Server
    unsigned char *minecraft = *(unsigned char **) (screen + Screen_minecraft_property_offset);
    unsigned char *rak_net_instance = *(unsigned char **) (minecraft + Minecraft_rak_net_instance_property_offset);
    if (rak_net_instance != NULL) {
        unsigned char *rak_net_instance_vtable = *(unsigned char**) rak_net_instance;
        RakNetInstance_isServer_t RakNetInstance_isServer = *(RakNetInstance_isServer_t *) (rak_net_instance_vtable + RakNetInstance_isServer_vtable_offset);
        if ((*RakNetInstance_isServer)(rak_net_instance)) {
            // Add Button
            std::vector<unsigned char *> *rendered_buttons = (std::vector<unsigned char *> *) (screen + Screen_rendered_buttons_property_offset);
            std::vector<unsigned char *> *selectable_buttons = (std::vector<unsigned char *> *) (screen + Screen_selectable_buttons_property_offset);
            unsigned char *button = *(unsigned char **) (screen + PauseScreen_server_visibility_button_property_offset);
            rendered_buttons->push_back(button);
            selectable_buttons->push_back(button);

            // Update Button Text
            (*PauseScreen_updateServerVisibilityText)(screen);
        }
    }
}

// Init
void _init_misc_cpp() {
    // Implement AppPlatform::readAssetFile So Translations Work
    if (feature_has("Load Language Files", server_enabled)) {
        overwrite((void *) AppPlatform_readAssetFile, (void *) AppPlatform_readAssetFile_injection);
    }

    // Handle Custom Update Behavior
    overwrite_calls((void *) Minecraft_update, (void *) Minecraft_update_injection);

    // Fix Pause Menu
    if (feature_has("Fix Pause Menu", server_disabled)) {
        // Add Missing Buttons To Pause Menu
        patch_address(PauseScreen_init_vtable_addr, (void *) PauseScreen_init_injection);
    }
}
