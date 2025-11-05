#include <dirent.h>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/env/env.h>

#include "bootstrap.h"
#include "../util/util.h"

// Get All Mods In Folder
static void load(std::vector<std::string> &ld_preload, const std::string &folder, int recursion_limit = 128);
static void handle_file(std::vector<std::string> &ld_preload, const std::string &file, const bool is_dir, const int recursion_limit) {
    // Check If File Is A Symbolic Link
#ifdef _WIN32
    const DWORD attr = GetFileAttributesA(file.c_str());
    const bool is_symlink = (attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
#else
    struct stat file_stat = {};
    const int ret = lstat(file.c_str(), &file_stat);
    const bool is_symlink = ret == 0 && S_ISLNK(file_stat.st_mode);
#endif

    // Check Type
    if (is_dir) {
        // Recurse Into Directory
        load(ld_preload, std::string(file) + path_separator, recursion_limit - 1);
    } else if (is_symlink) {
        // Resolve Symlink
        const std::string resolved_file = safe_realpath(file);
        handle_file(ld_preload, resolved_file, is_dir, recursion_limit);
    } else {
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
    read_directory(folder, [&folder, &ld_preload, &recursion_limit](const dirent *entry, const bool is_dir) {
        // Get Full Name
        const std::string name = folder + entry->d_name;
        // Handle
        handle_file(ld_preload, name, is_dir, recursion_limit);
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
    for (std::string mods_folder : folders) {
        mods_folder += path_separator + std::string("mods") + path_separator;
        load(preload, mods_folder);
    }

    // Return
    return preload;
}
