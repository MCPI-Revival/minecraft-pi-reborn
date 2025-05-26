#include <cstdlib>

#include <libreborn/log.h>
#include <libreborn/config.h>
#include <libreborn/env/env.h>
#include <libreborn/env/flags.h>

#include <mods/feature/feature.h>

// Flags In Server-Mode
bool feature_server_flags_set = false;
#define FLAG(name) bool server_##name = false
#include <mods/feature/server.h>
#undef FLAG

// Check For Feature
bool feature_has(const char *name, const bool enabled_in_server_mode) {
    // Server Handling
    if (reborn_is_server()) {
        if (!feature_server_flags_set) {
            IMPOSSIBLE();
        }
        return enabled_in_server_mode;
    }

    // Load Flags
    static Flags flags = Flags::get();
    static bool loaded = false;
    if (!loaded) {
        const char *env = require_env(MCPI_FEATURE_FLAGS_ENV);
        env_value_to_obj(flags, env);
        loaded = true;
    }

    // Get Value
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
