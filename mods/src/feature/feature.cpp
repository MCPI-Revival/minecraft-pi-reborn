#include <cstdlib>
#include <cstring>

#include <libreborn/log.h>
#include <libreborn/util.h>
#include <libreborn/env.h>
#include <libreborn/flags.h>

#include <mods/feature/feature.h>

// Check For Feature
bool _feature_has(const char *name, const int server_default) {
    // Server Handling
    if (reborn_is_server() && server_default != -1) {
        return server_default > 0;
    }
    // Get Value
    static Flags flags = Flags::get();
    static bool loaded = false;
    if (!loaded) {
        const char *env = getenv(MCPI_FEATURE_FLAGS_ENV);
        if (env) {
            flags = env;
        }
        loaded = true;
    }
    int ret = -1;
    flags.root.for_each_const([&ret, &name](const FlagNode &flag) {
        if (flag.name == std::string(name)) {
            ret = flag.value;
        }
    });
    if (ret == -1) {
        ERR("Invalid Feature Flag: %s", name);
    }

    // Log
    DEBUG("Feature: %s: %s", name, ret ? "Enabled" : "Disabled");

    // Return
    return ret;
}
