#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/home/home.h>
#include <mods/init/init.h>

// Get MCPI Home Directory
const char *home_get() {
    static std::string dir = "";
    // Load
    if (dir.empty()) {
        dir = std::string(getenv(_MCPI_HOME_ENV)) + std::string(get_home_subdirectory_for_game_data());
    }
    // Return
    return dir.c_str();
}

// Use MCPI_HOME
static const char *getenv_HOME(__attribute__((unused)) const char *env) {
    return getenv(_MCPI_HOME_ENV);
}

// Init
void init_home() {
    // Store Data In ~/.minecraft-pi Instead Of ~/.minecraft
    patch_address(&Strings::default_path, (void *) get_home_subdirectory_for_game_data());
    // Use MCPI_HOME Instead Of $HOME
    overwrite_call((void *) 0xe0e4, (void *) getenv_HOME);

    // The override code resolves assets manually,
    // making changing directory redundant.
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0xe0ac, nop_patch);
}
