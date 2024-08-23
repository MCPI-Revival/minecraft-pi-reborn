#include <dirent.h>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

#include <libreborn/libreborn.h>

#include "bootstrap.h"

// Get All Mods In Folder
static void load(std::string &ld_preload, const std::string &folder, int recursion_limit = 128);
static void handle_file(std::string &ld_preload, const std::string &file, const int recursion_limit) {
    // Check Type
    struct stat file_stat = {};
    lstat(file.c_str(), &file_stat);
    if (S_ISDIR(file_stat.st_mode)) {
        // Recurse Into Directory
        load(ld_preload, std::string(file) + "/", recursion_limit - 1);
    } else if (S_ISLNK(file_stat.st_mode)) {
        // Resolve Symlink
        char *resolved_file = realpath(file.c_str(), nullptr);
        ALLOC_CHECK(resolved_file);
        handle_file(ld_preload, resolved_file, recursion_limit);
        free(resolved_file);
    } else if (S_ISREG(file_stat.st_mode)) {
        // Check If File Is Accessible
        const int result = access(file.c_str(), R_OK);
        if (result == 0) {
            // Add To LD_PRELOAD
            ld_preload += file + ":";
        } else if (result == -1 && errno != 0) {
            // Fail
            WARN("Unable To Access: %s: %s", file.c_str(), strerror(errno));
            errno = 0;
        }
    }
}
static void load(std::string &ld_preload, const std::string &folder, const int recursion_limit) {
    // Check Recursion
    if (recursion_limit <= 0) {
        ERR("Reached Recursion Limit While Loading Mods");
    }
    // Open Folder
    ensure_directory(folder.c_str());
    DIR *dp = opendir(folder.c_str());
    if (dp == nullptr) {
        // Unable To Open Folder
        ERR("Error Opening Directory: %s: %s", folder.c_str(), strerror(errno));
    }
    // Loop Through Folder
    while (true) {
        errno = 0;
        const dirent *entry = readdir(dp);
        if (entry != nullptr) {
            // Block Pseudo-Directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            // Get Full Name
            std::string name = folder + entry->d_name;
            // Handle
            handle_file(ld_preload, name, recursion_limit);
        } else if (errno != 0) {
            // Error Reading Contents Of Folder
            ERR("Error Reading Directory: %s: %s", folder.c_str(), strerror(errno));
        } else {
            // Done!
            break;
        }
    }
    // Close Folder
    closedir(dp);
}

// Bootstrap Mods
#define SUBDIRECTORY_FOR_MODS "/mods/"
std::string bootstrap_mods(const std::string &binary_directory) {
    // Prepare
    std::string preload = "";

    // ~/.minecraft-pi/mods
    {
        // Get Mods Folder
        const std::string mods_folder = std::string(getenv(_MCPI_HOME_ENV)) + get_home_subdirectory_for_game_data() + SUBDIRECTORY_FOR_MODS;
        // Load Mods From ./mods
        load(preload, mods_folder);
    }

    // Built-In Mods
    {
        // Get Mods Folder
        const std::string mods_folder = binary_directory + SUBDIRECTORY_FOR_MODS;
        // Load Mods From ./mods
        load(preload, mods_folder);
    }

    // Return
    return preload;
}
