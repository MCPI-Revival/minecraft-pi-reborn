#include <dirent.h>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

#include <libreborn/log.h>
#include <libreborn/util/util.h>

#include "bootstrap.h"
#include "../util/util.h"

// Get All Mods In Folder
static void load(std::vector<std::string> &ld_preload, const std::string &folder, int recursion_limit = 128);
static void handle_file(std::vector<std::string> &ld_preload, const std::string &file, const int recursion_limit) {
    // Check Type
    struct stat file_stat = {};
    lstat(file.c_str(), &file_stat);
    if (S_ISDIR(file_stat.st_mode)) {
        // Recurse Into Directory
        load(ld_preload, std::string(file) + '/', recursion_limit - 1);
    } else if (S_ISLNK(file_stat.st_mode)) {
        // Resolve Symlink
        const std::string resolved_file = safe_realpath(file);
        handle_file(ld_preload, resolved_file, recursion_limit);
    } else if (S_ISREG(file_stat.st_mode)) {
        // Check If File Is Accessible
        const int result = access(file.c_str(), R_OK);
        if (result == 0) {
            // Add To LD_PRELOAD
            DEBUG("Found Mod: %s", file.c_str());
            ld_preload.push_back(file);
        } else if (result == -1 && errno != 0) {
            // Fail
            WARN("Unable To Access: %s: %s", file.c_str(), strerror(errno));
            errno = 0;
        }
    }
}
static void load(std::vector<std::string> &ld_preload, const std::string &folder, const int recursion_limit) {
    // Check Recursion
    if (recursion_limit <= 0) {
        ERR("Reached Recursion Limit While Loading Mods");
    }
    // Make Directory
    ensure_directory(folder.c_str());
    // Read
    read_directory(folder, [&folder, &ld_preload, &recursion_limit](const dirent *entry) {
        // Get Full Name
        const std::string name = folder + entry->d_name;
        // Handle
        handle_file(ld_preload, name, recursion_limit);
    });
}

// Bootstrap Mods
#define SUBDIRECTORY_FOR_MODS "/mods/"
std::vector<std::string> bootstrap_mods(const std::string &binary_directory) {
    // Prepare
    std::vector<std::string> preload;

    // Load
    const std::vector folders = {
        home_get(),
        binary_directory
    };
    for (std::string mods_folder : folders) {
        mods_folder += SUBDIRECTORY_FOR_MODS;
        load(preload, mods_folder);
    }

    // Return
    return preload;
}
