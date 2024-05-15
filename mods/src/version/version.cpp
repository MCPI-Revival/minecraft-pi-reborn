#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/version/version.h>
#include <mods/init/init.h>

// Get New Version
char *version_get() {
    static char *version = nullptr;
    // Load
    if (version == nullptr) {
        safe_asprintf(&version, "%s / Reborn v%s", Strings_minecraft_pi_version, reborn_get_version());
    }
    // Return
    return version;
}
// Free
__attribute__((destructor)) static void _free_version() {
    free(version_get());
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
    patch_address((void *) Strings_minecraft_pi_version_pointer, version_get());

    // Log
    INFO("Starting Minecraft: Pi Edition (%s)", version_get());
}
