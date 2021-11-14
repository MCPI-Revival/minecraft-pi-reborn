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

// Init
void _init_misc_cpp() {
    // Implement AppPlatform::readAssetFile So Translations Work
    if (feature_has("Load Language Files", 1)) {
        overwrite((void *) AppPlatform_readAssetFile, (void *) AppPlatform_readAssetFile_injection);
    }

    // Handle Custom Update Behavior
    overwrite_calls((void *) Minecraft_update, (void *) Minecraft_update_injection);
}
