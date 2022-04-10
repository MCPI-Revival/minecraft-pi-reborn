#include <stdlib.h>
#include <string.h>

#include <libreborn/libreborn.h>

#include "feature.h"

// Check For Feature
int _feature_has(const char *name) {
    // Get Value
    char *env = getenv("MCPI_FEATURE_FLAGS");
    char *features = strdup(env != NULL ? env : "");
    char *tok = strtok(features, "|");
    int ret = 0;
    while (tok != NULL) {
        if (strcmp(tok, name) == 0) {
            ret = 1;
            break;
        }
        tok = strtok(NULL, "|");
    }
    free(features);

    // Log
    DEBUG("Feature: %s: %s", name, ret ? "Enabled" : "Disabled");

    // Return
    return ret;
}
