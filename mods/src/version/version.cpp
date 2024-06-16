#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/version/version.h>
#include <mods/init/init.h>

// Get New Version
const char *version_get() {
    static std::string version = "";
    // Load
    if (version.empty()) {
        version = std::string(Strings::minecraft_pi_version) + " / Reborn v" + reborn_get_version();
    }
    // Return
    return version.c_str();
}

// Injection For Touch GUI Version
static std::string Common_getGameVersionString_injection(__attribute__((unused)) std::string *version_suffix) {
    // Set Version
    return version_get();
}

// Init
void init_version() {
    // Touch GUI
    overwrite(Common_getGameVersionString, Common_getGameVersionString_injection);
    // Normal GUI
    patch_address((void *) &Strings::minecraft_pi_version, (void *) version_get());

    // Log
    INFO("Starting Minecraft: Pi Edition (%s)", version_get());
}
