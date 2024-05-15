#include <stdlib.h>
#include <string.h>

#include <libreborn/libreborn.h>

#include <mods/feature/feature.h>

// Check For Feature
bool _feature_has(const char *name) {
    // Get Value
    char *env = getenv("MCPI_FEATURE_FLAGS");
    char *features = strdup(env != nullptr ? env : "");
    char *tok = strtok(features, "|");
    bool ret = false;
    while (tok != nullptr) {
        if (strcmp(tok, name) == 0) {
            ret = true;
            break;
        }
        tok = strtok(nullptr, "|");
    }
    free(features);

    // Log
    DEBUG("Feature: %s: %s", name, ret ? "Enabled" : "Disabled");

    // Return
    return ret;
}

