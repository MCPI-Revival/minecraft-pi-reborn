#include <stdlib.h>
#include <string.h>

#include <libcore/libcore.h>

#include "feature.h"

// Check For Feature
int feature_has(const char *name) {
    char *env = getenv("MCPI_FEATURES");
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
    if (feature_get_mode() != 2) {
        INFO("Feature: %s: %s", name, ret ? "Enabled" : "Disabled");
    }
    return ret;
}

// Get Graphics Mode
int feature_get_mode() {
    char *mode = getenv("MCPI_MODE");
    if (mode == NULL) {
        ERR("%s", "MCPI Mode Not Specified");
    } else if (strcmp("virgl", mode) == 0) {
        return 0;
    } else if (strcmp("native", mode) == 0) {
        return 1;
    } else if (strcmp("server", mode) == 0) {
        return 2;
    } else {
        ERR("Inavlid MCPI_MODE: %s", mode);
    }
}
