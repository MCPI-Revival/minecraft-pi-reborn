#include <unistd.h>

#include "util.h"

#include <libreborn/log.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>

// Profile Directory
void setup_home() {
    const char *custom_profile_directory = getenv(MCPI_PROFILE_DIRECTORY_ENV);
    std::string home;
    if (custom_profile_directory != nullptr) {
        // Custom Directory
        home = custom_profile_directory;
    } else if (!reborn_is_server()) {
        // Ensure $HOME
        const char *env =
#ifdef _WIN32
            "APPDATA"
#else
            "HOME"
#endif
            ;
        home = require_env(env);
        // Flatpak
        if (reborn_config.packaging == RebornConfig::PackagingType::FLATPAK) {
            home += std::string("/.var/app/") + reborn_config.app.id;
        }
    } else {
        // Set Home To The Current Directory, So World Data Is Stored There
        char *launch_directory = getcwd(nullptr, 0);
        if (launch_directory == nullptr) {
            IMPOSSIBLE();
        }
        home = launch_directory;
        free(launch_directory);
    }
    home = safe_realpath(home);
    // Set
    set_and_print_env(_MCPI_HOME_ENV, home.c_str());
}