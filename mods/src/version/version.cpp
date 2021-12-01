#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/version/version.h>
#include <mods/init/init.h>

// Get New Version
char *version_get() {
    static char *version = NULL;
    // Load
    if (version == NULL) {
        safe_asprintf(&version, "%s / Reborn v" MCPI_VERSION, *minecraft_pi_version);
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
    unsigned char normal_gui_version_length_patch[4] = {(unsigned char) strlen(version_get()), 0x20, 0xa0, 0xe3}; // "mov r2, #VERSION_LENGTH"
    patch((void *) 0x4b11c, normal_gui_version_length_patch);

    // Log
    INFO("Starting Minecraft: Pi Edition (%s)", version_get());
}
