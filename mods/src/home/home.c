#include <libreborn/libreborn.h>
#include <libreborn/minecraft.h>

#include "home.h"
#include "../init/init.h"

// Minecraft Pi User Data Root
#ifndef MCPI_SERVER_MODE
// Store Game Data In "~/.minecraft-pi" Instead Of "~/.minecraft" To Avoid Conflicts
#define NEW_PATH "/.minecraft-pi"
#else
// Store Game Data In Launch Directory
#define NEW_PATH ""

// Store Launch Directory
static char *launch_directory = NULL;
__attribute__((constructor)) static void init_launch_directory() {
    launch_directory = getcwd(NULL, 0);
}

// Pretend $HOME Is Launch Directory
HOOK(getenv, char *, (const char *name)) {
    if (strcmp(name, "HOME") == 0) {
        return launch_directory;
    } else {
        ensure_getenv();
        return (*real_getenv)(name);
    }
}
#endif

// Get MCPI Home Directory
char *home_get() {
    static char *dir = NULL;
    // Load
    if (dir == NULL) {
        safe_asprintf(&dir, "%s/" NEW_PATH, getenv("HOME"));
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
    patch_address((void *) default_path, (void *) NEW_PATH);
}
