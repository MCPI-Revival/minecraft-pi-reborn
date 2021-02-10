#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>

#include <libreborn/libreborn.h>

static int starts_with(const char *s, const char *t) {
    return strncmp(s, t, strlen(t)) == 0;
}

static char *get_override_path(const char *filename) {
    // Get Asset Override Path
    char *overrides = NULL;
    asprintf(&overrides, "%s/.minecraft-pi/overrides", getenv("HOME"));
    // Get data Path
    char *data = NULL;
    char *cwd = getcwd(NULL, 0);
    asprintf(&data, "%s/data", cwd);
    free(cwd);
    // Get Full Path
    char *new_path = NULL;
    char *full_path = realpath(filename, NULL);
    if (full_path != NULL) {
        if (starts_with(full_path, data)) {
            asprintf(&new_path, "%s%s", overrides, &full_path[strlen(data)]);
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
    char *new_path = get_override_path(filename);
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
    char *new_path = get_override_path(filename);
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