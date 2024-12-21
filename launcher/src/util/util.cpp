#include <dirent.h>
#include <cstring>

#include <libreborn/log.h>
#include <libreborn/util/exec.h>

#include "util.h"

// Chop Off Last Component
void chop_last_component(std::string &str) {
    const std::string::size_type pos = str.find_last_of('/');
    if (pos == std::string::npos) {
        return;
    }
    str = str.substr(0, pos);
}

// Get Binary Directory (Remember To Free)
std::string safe_realpath(const std::string &path) {
    char *raw = realpath(path.c_str(), nullptr);
    if (raw == nullptr) {
        ERR("Unable To Resolve: %s", path.c_str());
    }
    std::string str = raw;
    free(raw);
    return str;
}
std::string get_binary_directory() {
    // Get Path To Current Executable
    std::string exe = safe_realpath("/proc/self/exe");
    // Chop Off Last Component
    chop_last_component(exe);
    // Return
    return exe;
}

// Read Directory
bool read_directory(const std::string &path, const std::function<void(const dirent *)> &callback, const bool allow_nonexistent_dir) {
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
        // Run
        callback(entry);
    }
    // Close
    closedir(dp);
    return true;
}