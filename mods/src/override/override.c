#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>

#include <libreborn/libreborn.h>

#include "override.h"
#include "../home/home.h"

// Hook access
HOOK(access, int, (const char *pathname, int mode)) {
    char *new_path = override_get_path(pathname);
    // Open File
    ensure_access();
    int ret = (*real_access)(new_path != NULL ? new_path : pathname, mode);
    // Free Data
    if (new_path != NULL) {
        free(new_path);
    }
    // Return
    return ret;
}

// Get Override Path For File (If It Exists)
char *override_get_path(const char *filename) {
    // Get MCPI Home Path
    char *home_path = home_get();
    // Get Asset Override Path
    char *overrides = NULL;
    safe_asprintf(&overrides, "%s/overrides", home_path);

    // Data Prefiix
    char *data_prefix = "data/";
    int data_prefix_length = strlen(data_prefix);

    // Folders To Check
    char *asset_folders[] = {
        overrides,
        getenv("MCPI_REBORN_ASSETS_PATH"),
        getenv("MCPI_VANILLA_ASSETS_PATH"),
        NULL
    };

    // Check For Override
    char *new_path = NULL;
    if (starts_with(filename, data_prefix)) {
        // Test Asset Folders
        for (int i = 0; asset_folders[i] != NULL; i++) {
            safe_asprintf(&new_path, "%s/%s", asset_folders[i], &filename[data_prefix_length]);
            ensure_access();
            if ((*real_access)(new_path, F_OK) == -1) {
                // Not Found In Asset Folder
                free(new_path);
                new_path = NULL;
                continue;
            } else {
                // Found
                break;
            }
        }
    }

    // Free
    free(overrides);

    // Return
    return new_path;
}

// Hook fopen
HOOK(fopen, FILE *, (const char *filename, const char *mode)) {
    char *new_path = override_get_path(filename);
    // Open File
    ensure_fopen();
    FILE *file = (*real_fopen)(new_path != NULL ? new_path : filename, mode);
    // Free Data
    if (new_path != NULL) {
        free(new_path);
    }
    // Return File
    return file;
}

// Hook fopen64
HOOK(fopen64, FILE *, (const char *filename, const char *mode)) {
    char *new_path = override_get_path(filename);
    // Open File
    ensure_fopen64();
    FILE *file = (*real_fopen64)(new_path != NULL ? new_path : filename, mode);
    // Free Data
    if (new_path != NULL) {
        free(new_path);
    }
    // Return File
    return file;
}
