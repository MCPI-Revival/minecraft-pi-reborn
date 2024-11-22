#include "util.h"

#include <libreborn/libreborn.h>

// $PATH
void setup_path() {
    // Get Binary Directory
    const std::string binary_directory = get_binary_directory();
    std::string new_path = binary_directory + "/bin";
    // Add Existing PATH
    const char *value = getenv("PATH");
    if (value != nullptr && strlen(value) > 0) {
        new_path += std::string(":") + value;
    }
    // Set And Free
    set_and_print_env("PATH", new_path.c_str());
}

// Profile Directory
void setup_home() {
    const char *custom_profile_directory = getenv(MCPI_PROFILE_DIRECTORY_ENV);
    std::string home;
    if (custom_profile_directory != nullptr) {
        // Custom Directory
        home = safe_realpath(custom_profile_directory);
    } else if (!reborn_is_server()) {
        // Ensure $HOME
        const char *value = getenv("HOME");
        if (value == nullptr) {
            ERR("$HOME Is Not Set");
        }
        home = value;
        // Flatpak
#ifdef MCPI_IS_FLATPAK_BUILD
        home += "/.var/app/" MCPI_APP_ID;
#endif
    } else {
        // Set Home To Current Directory, So World Data Is Stored There
        char *launch_directory = getcwd(nullptr, 0);
        ALLOC_CHECK(launch_directory);
        home = launch_directory;
        free(launch_directory);
    }
    // Set
    set_and_print_env(_MCPI_HOME_ENV, home.c_str());
}