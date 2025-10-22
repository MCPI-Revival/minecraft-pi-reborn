#include <libreborn/env/env.h>

#include "parser.h"

// No Option Parsing On Windows
options_t parse_options(MCPI_UNUSED const int argc, MCPI_UNUSED char *argv[]) {
    // Force Log Settings
    for (const char *env : {MCPI_DEBUG_ENV, MCPI_QUIET_ENV}) {
        setenv_safe(env, nullptr);
    }
    // Create Options
    options_t options = {};
    options.debug = true;
    return options;
}