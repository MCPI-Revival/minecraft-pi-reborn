#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "version.h"
#include "../init/init.h"

// Get New Version
char *version_get() {
    static char *version = NULL;
    // Load
    if (version == NULL) {
        safe_asprintf(&version, "%s / Reborn v" VERSION, *minecraft_pi_version);
    }
    // Return
    return version;
}
// Free
__attribute__((destructor)) static void _free_version() {
    free(version_get());
}

// Injection For Touch GUI Version
static std::string Common_getGameVersionString_injection(__attribute__((unused)) std::string const& version_suffix) {
    // Set Version
    return version_get();
}

// Init
void init_version() {
    // Touch GUI
    overwrite((void *) Common_getGameVersionString, (void *) Common_getGameVersionString_injection);
    // Normal GUI
    patch_address((void *) minecraft_pi_version, version_get());

    // Log
    INFO("Starting Minecraft: Pi Edition (%s)", version_get());
}
