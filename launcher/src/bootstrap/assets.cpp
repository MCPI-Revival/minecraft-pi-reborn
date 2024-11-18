#include <libreborn/libreborn.h>

#include "bootstrap.h"
#include "../util/util.h"

// Setup Asset Paths
static void setup_path(const char *env_name, std::string assets_path) {
    chop_last_component(assets_path);
    assets_path += "/data";
    set_and_print_env(env_name, assets_path.c_str());
}
void bootstrap_assets(const std::string &original_game_binary) {
    setup_path(_MCPI_REBORN_ASSETS_PATH_ENV, safe_realpath("/proc/self/exe"));
    setup_path(_MCPI_VANILLA_ASSETS_PATH_ENV, original_game_binary);
}