#include "util.h"

#include <dirent.h>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <sys/stat.h>
#else
#include <cstring>
#include <unistd.h>
#endif

#include <libreborn/log.h>
#include <libreborn/util/io.h>
#include <libreborn/util/util.h>

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

// Utility Functions
void make_directory(std::string path /* Must Be Absolute */) {
    if (!path.ends_with(path_separator)) {
        path += path_separator;
    }
    std::stringstream stream(path);
    path = "";
    std::string path_segment;
    while (std::getline(stream, path_segment, path_separator)) {
        path += path_segment;
        ensure_directory(path.c_str());
        path += path_separator;
    }
}
void delete_recursively(const std::string &path, const bool allow_nonexistent_dir) {
    // Loop Through Children
    const bool success = read_directory(path, [&path](const dirent *entry, const bool is_dir) {
        // Handle
        const std::string child = path + path_separator + entry->d_name;
        if (is_dir) {
            delete_recursively(child, false);
        } else if (unlink(child.c_str()) != 0) {
            ERR("Unable To Delete File: %s: %s", child.c_str(), strerror(errno));
        }
    }, allow_nonexistent_dir);
    // Delete
    if (success && rmdir(path.c_str()) != 0) {
        ERR("Unable To Delete Directory: %s: %s", path.c_str(), strerror(errno));
    }
}
void copy_file(const std::string &src, const std::string &dst, const bool log) {
    std::ifstream in(src, std::ios::binary);
    if (!in) {
        ERR("Unable To Open Source File: %s", src.c_str());
    }
    std::ofstream out(dst, std::ios::binary);
    if (!out) {
        ERR("Unable To Create Destination File: %s", dst.c_str());
    }
    out << in.rdbuf();
    out.close();
    in.close();
    if (log) {
        INFO("Installed: %s", dst.c_str());
    }
}
void copy_directory(const std::string &src, const std::string &dst) {
    read_directory(src, [&src, &dst](const dirent *entry, const bool is_dir) {
        const std::string name = path_separator + std::string(entry->d_name);
        const std::string in = src + name;
        const std::string out = dst + name;
        if (is_dir) {
            ensure_directory(out.c_str());
            copy_directory(in, out);
        } else {
            copy_file(in, out);
        }
    });
}