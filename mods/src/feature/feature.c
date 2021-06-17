#include <stdlib.h>
#include <string.h>

#include <libreborn/libreborn.h>

#include "feature.h"

// Check For Feature
int feature_has(const char *name) {
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
#ifndef MCPI_SERVER_MODE
    INFO("Feature: %s: %s", name, ret ? "Enabled" : "Disabled");
#endif
    return ret;
}
