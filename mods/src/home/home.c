#include <errno.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "home.h"
#include "../init/init.h"

// Get MCPI Home Directory
char *home_get() {
    static char *dir = NULL;
    // Load
    if (dir == NULL) {
        safe_asprintf(&dir, "%s" HOME_SUBDIRECTORY_FOR_GAME_DATA, getenv("HOME"));
    }
    // Return
    return dir;
}
// Free
__attribute__((destructor)) static void _free_home() {
    free(home_get());
}

// Init
void init_home() {
    // Store Data In ~/.minecraft-pi Instead Of ~/.minecraft
    patch_address((void *) default_path, (void *) HOME_SUBDIRECTORY_FOR_GAME_DATA);

    // The override code resolves assets manually,
    // making changing directory redundant.
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0xe0ac, nop_patch);
}
