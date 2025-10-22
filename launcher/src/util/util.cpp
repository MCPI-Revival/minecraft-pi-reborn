#include <dirent.h>
#include <cstring>
#include <sys/stat.h>

#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/env/env.h>

#include "util.h"

// Chop Off Last Component
void chop_last_component(std::string &str) {
    const std::string::size_type pos = str.find_last_of(path_separator);
    if (pos == std::string::npos) {
        return;
    }
    str = str.substr(0, pos);
}

// Get Binary Directory (Remember To Free)
std::string safe_realpath(const std::string &path) {
#ifdef _WIN32
    char *raw = _fullpath(nullptr, path.c_str(), MAX_PATH);
#else
    char *raw = realpath(path.c_str(), nullptr);
#endif
    if (raw == nullptr) {
        ERR("Unable To Resolve: %s", path.c_str());
    }
    std::string str = raw;
    free(raw);
    return str;
}
std::string get_binary() {
#ifdef _WIN32
    constexpr int binary_directory_max_length = MAX_PATH;
    char binary_directory_raw[binary_directory_max_length] = {};
    const bool ret = GetModuleFileNameA(nullptr, binary_directory_raw, binary_directory_max_length);
    if (!ret) {
        ERR("Unable To Locate Binary");
    }
    return binary_directory_raw;
#else
    return safe_realpath("/proc/self/exe");
#endif
}
std::string get_binary_directory() {
    // Get Path To Current Executable
    std::string exe = get_binary();
    // Chop Off Last Component
    chop_last_component(exe);
    // Return
    return exe;
}

// AppImage
std::string get_appimage_path() {
    return require_env("APPIMAGE");
}

// Read Directory
bool read_directory(const std::string &path, const std::function<void(const dirent *, bool)> &callback, const bool allow_nonexistent_dir) {
    // Open Directory
    DIR *dp = opendir(path.c_str());
    if (dp == nullptr) {
        if (allow_nonexistent_dir) {
            return false;
        }
        ERR("Unable To Open Directory: %s: %s", path.c_str(), strerror(errno));
    }

    // Read
    const dirent *entry;
    while ((entry = readdir(dp)) != nullptr) {
        // Block Pseudo-Directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // Check If Entry Is A Directory
#ifdef _WIN32
        bool is_dir = false;
        const std::string file = path + path_separator + entry->d_name;
        struct stat obj = {};
        if (stat(file.c_str(), &obj) == 0) {
            is_dir = S_ISDIR(obj.st_mode);
        }
#else
        const bool is_dir = entry->d_type == DT_DIR;
#endif
        // Run
        callback(entry, is_dir);
    }
    // Close
    closedir(dp);
    return true;
}