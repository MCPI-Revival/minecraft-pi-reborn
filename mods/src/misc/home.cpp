#include <libreborn/patch.h>
#include <libreborn/env/env.h>
#include <libreborn/util/util.h>

#include <symbols/minecraft.h>

#include "misc-internal.h"

// Use MCPI_HOME
static const char *getenv_HOME(__attribute__((unused)) const char *env) {
    return getenv(_MCPI_HOME_ENV);
}

// Init
void _init_misc_home() {
    // Store Data In ~/.minecraft-pi Instead Of ~/.minecraft
    patch_address(&Strings::default_path, (void *) get_home_subdirectory_for_game_data());
    // Use MCPI_HOME Instead Of $HOME
    overwrite_call((void *) 0xe0e4, (void *) getenv_HOME);

    // The override code resolves assets manually,
    // making changing directory redundant.
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0xe0ac, nop_patch);
}
