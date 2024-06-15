#include <dirent.h>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

#include <libreborn/libreborn.h>

#include "bootstrap.h"

// Get All Mods In Folder
static void load(std::string &ld_preload, const std::string &folder) {
    // Open Folder
    DIR *dp = opendir(folder.c_str());
    if (dp != nullptr) {
        // Loop Through Folder
        while (true) {
            errno = 0;
            dirent *entry = readdir(dp);
            if (entry != nullptr) {
                // Check If File Is Regular
                if (entry->d_type == DT_REG) {
                    // Get Full Name
                    std::string name = folder + entry->d_name;

                    // Check If File Is Accessible
                    int result = access(name.c_str(), R_OK);
                    if (result == 0) {
                        // Add To LD_PRELOAD
                        ld_preload += name + ":";
                    } else if (result == -1 && errno != 0) {
                        // Fail
                        WARN("Unable To Access: %s: %s", name.c_str(), strerror(errno));
                        errno = 0;
                    }
                }
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
    } else if (errno == ENOENT) {
        // Folder Doesn't Exist
    } else {
        // Unable To Open Folder
        ERR("Error Opening Directory: %s: %s", folder.c_str(), strerror(errno));
    }
}

// Bootstrap Mods
#define SUBDIRECTORY_FOR_MODS "/mods/"
std::string bootstrap_mods(const std::string &binary_directory) {
    // Prepare
    std::string preload = "";

    // ~/.minecraft-pi/mods
    {
        // Get Mods Folder
        std::string mods_folder = std::string(getenv("HOME")) + get_home_subdirectory_for_game_data() + SUBDIRECTORY_FOR_MODS;
        // Load Mods From ./mods
        load(preload, mods_folder);
    }

    // Built-In Mods
    {
        // Get Mods Folder
        std::string mods_folder = binary_directory + SUBDIRECTORY_FOR_MODS;
        // Load Mods From ./mods
        load(preload, mods_folder);
    }

    // Return
    return preload;
}
