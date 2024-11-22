#include <cstdlib>

#include <libreborn/config.h>
#include <libreborn/env.h>

// Access Configuration At Runtime
const char *reborn_get_version() {
    return MCPI_VERSION;
}
bool reborn_is_headless() {
    static bool ret;
    static bool is_set = false;
    if (!is_set) {
        ret = reborn_is_server();
        if (getenv(_MCPI_FORCE_HEADLESS_ENV)) {
            ret = true;
        } else if (getenv(_MCPI_FORCE_NON_HEADLESS_ENV)) {
            ret = false;
        }
        is_set = true;
    }
    return ret;
}
bool reborn_is_server() {
    static int ret;
    static bool is_set = false;
    if (!is_set) {
        ret = getenv(_MCPI_SERVER_MODE_ENV) != nullptr;
        is_set = true;
    }
    return ret;
}