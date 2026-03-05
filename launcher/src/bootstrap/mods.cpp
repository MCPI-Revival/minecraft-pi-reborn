#include <dirent.h>
#include <cerrno>
#include <sys/stat.h>
#include <cstring>
#include <unordered_set>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/env/env.h>

#include "bootstrap.h"
#include "../util/util.h"

// Handle A Single Mod
static void handle_file(std::vector<std::string> &ld_preload, const std::string &file) {
    // Check If File Is Accessible
    const int result = access(file.c_str(), R_OK);
    if (result == 0) {
        // Add To LD_PRELOAD
        if (file.ends_with(".so")) {
            DEBUG("Found Mod: %s", file.c_str());
            ld_preload.push_back(file);
        }
    } else if (result == -1) {
        // Fail
        WARN("Unable To Access: %s: %s", file.c_str(), strerror(errno));
    }
}

// Get All Mods In Folder
static constexpr int recursion_limit = 64;
static void load(std::vector<std::string> &ld_preload, const std::string &folder, std::unordered_set<std::string> &visited, const int recursion_level) {
    // Check Folder
    if (recursion_level >= recursion_limit) {
        ERR("Reached Recursion Limit While Loading Mods");
    } else if (visited.contains(folder)) {
        return;
    }
    visited.insert(folder);

    // Make Directory
    ensure_directory(folder.c_str());

    // Read
    read_directory(folder, [&folder, &ld_preload, &visited, &recursion_level](const dirent *entry, const bool is_dir) {
        // Get Full Name
        std::string name = folder + path_separator + entry->d_name;
        name = safe_realpath(name);
        // Handle
        if (is_dir) {
            // Recurse Into Directory
            load(ld_preload, name, visited, recursion_level + 1);
        } else {
            // Load Dile
            handle_file(ld_preload, name);
        }
    });
}

// Bootstrap Mods
std::vector<std::string> bootstrap_mods(const std::string &binary_directory) {
    // Prepare
    std::vector<std::string> preload;

    // Load
    std::vector<std::string> folders;
    if (!is_env_set(_MCPI_BENCHMARK_ENV)) {
        folders.push_back(home_get());
    }
    folders.push_back(binary_directory);
    std::unordered_set<std::string> visited;
    for (std::string mods_folder : folders) {
        mods_folder += path_separator;
        mods_folder += "mods";
        load(preload, mods_folder, visited, 0);
    }

    // Return
    return preload;
}
