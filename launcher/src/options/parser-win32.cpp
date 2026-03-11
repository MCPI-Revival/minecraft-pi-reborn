#include <getopt.h>
#include <limits>

#include <libreborn/env/env.h>
#include <libreborn/log.h>

#include "parser.h"

// Long Option
static constexpr int get_safe_val(int val) {
    if (val < 0) {
        // getopt_long() Has Issues With Negative Keys
        val *= -1;
        val += std::numeric_limits<unsigned char>::max() + 1;
    }
    return val;
}
static option long_options[] = {
#define OPTION(ignored, name, key, ...) {name, no_argument, nullptr, get_safe_val(key)},
#include "list.h"
#undef OPTION
    {nullptr, 0, nullptr, 0}
};

// Short Options
static std::string get_short_options() {
    std::string str;
#define OPTION(ignored1, ignored2, key, ...) if ((key) > 0) str += char(key);
#include "list.h"
#undef OPTION
    return str;
}

// Limited Option Parsing On Windows
options_t parse_options(MCPI_UNUSED const int argc, MCPI_UNUSED char *argv[]) {
    // Parse
    options_t options = {};
    while (true) {
        const std::string short_options = get_short_options();
        const int opt = getopt_long(argc, argv, short_options.c_str(), long_options, nullptr);
        if (opt == -1) {
            break;
        }
        switch (opt) {
#define OPTION(name, ignored, key, ...) case get_safe_val(key): options.name = true; break;
#include "list.h"
#undef OPTION
            default: ERR("Unable To Parse Options");
        }
    }

    // Force Log Settings
    for (const char *env : {MCPI_DEBUG_ENV, MCPI_QUIET_ENV}) {
        setenv_safe(env, nullptr);
    }
    options.debug = true;
    options.quiet = false;

    // Return
    return options;
}