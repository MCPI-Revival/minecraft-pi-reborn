#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>

#include <libreborn/libreborn.h>

#include "../home/home.h"

// Get Override Path For File (If It Exists)
char *override_get_path(const char *filename) {
    // Get MCPI Home Path
    char *home_path = home_get();
    // Get Asset Override Path
    char *overrides = NULL;
    safe_asprintf(&overrides, "%s/overrides", home_path);

    // Get Data Path
    char *data = NULL;
    char *binary_directory = get_mcpi_directory();
    safe_asprintf(&data, "%s/data", binary_directory);
    int data_length = strlen(data);

    // Get Full Path
    char *full_path;
    if (strlen(filename) > 0 && filename[0] == '/') {
        // Absolute Path
        full_path = strdup(filename);
    } else {
        // Relative Path
        full_path = realpath(filename, NULL);
    }

    // Check For Override
    char *new_path = NULL;
    if (full_path != NULL) {
        if (starts_with(full_path, data)) {
            safe_asprintf(&new_path, "%s%s", overrides, &full_path[data_length]);
            if (access(new_path, F_OK) == -1) {
                free(new_path);
                new_path = NULL;
            }
        }
        free(full_path);
    }

    // Free Variables
    free(overrides);
    free(data);

    // Return
    return new_path;
}

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
