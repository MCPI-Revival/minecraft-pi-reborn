#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <cstring>
#include <cmath>

#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/config.h>
#include <libreborn/env/env.h>
#include <libreborn/log.h>

// Align Number
int align_up(int x, const int alignment) {
    const int diff = x % alignment;
    if (diff > 0) {
        x += (alignment - diff);
    }
    return x;
}

// Safe Version Of pipe()
Pipe::Pipe(): read(-1), write(-1) {
    int out[2];
    if (pipe(out) != 0) {
        ERR("Unable To Create Pipe: %s", strerror(errno));
    }
    const_cast<int &>(read) = out[0];
    const_cast<int &>(write) = out[1];
}

// Check If Two Percentages Are Different Enough To Be Logged
#define SIGNIFICANT_PROGRESS 5
bool is_progress_difference_significant(const int32_t new_val, const int32_t old_val) {
    if (new_val != old_val) {
        if (new_val == -1 || old_val == -1) {
            return true;
        } else if (new_val == 0 || new_val == 100) {
            return true;
        } else {
            return new_val - old_val >= SIGNIFICANT_PROGRESS;
        }
    } else {
        return false;
    }
}

// Lock File
int lock_file(const char *file) {
    // Get New Path
    const std::string lock = std::string(file) + ".lock";
    file = lock.c_str();
    // Lock
    const int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0) {
        ERR("Unable To Open Lock File: %s: %s", file, strerror(errno));
    }
    if (flock(fd, LOCK_EX) == -1) {
        ERR("Unable To Lock File: %s: %s", file, strerror(errno));
    }
    DEBUG("Locked File: %s: %i", file, fd);
    return fd;
}
void unlock_file(const int fd) {
    if (flock(fd, LOCK_UN) == -1) {
        ERR("Unable To Unlock File: %i: %s", fd, strerror(errno));
    }
    close(fd);
    DEBUG("Unlocked File: %i", fd);
}

// Check $DISPLAY
void reborn_check_display() {
    if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY")) {
        ERR("No display attached! Make sure $DISPLAY or $WAYLAND_DISPLAY is set.");
    }
}

// Home Subdirectory
const char *get_home_subdirectory_for_game_data() {
    if (getenv(MCPI_PROFILE_DIRECTORY_ENV) != nullptr) {
        // No Subdirectory When Using Custom Profile Directory
        return "";
    } else if (!reborn_is_server()) {
        // Store Game Data In "~/.minecraft-pi" Instead Of "~/.minecraft" To Avoid Conflicts
        return "/.minecraft-pi";
    } else {
        // Store Game Data In $HOME Root (In Server Mode, $HOME Is Changed To The Launch Directory)
        return "";
    }
}

// Make Sure Directory Exists
void ensure_directory(const char *path) {
    if (path[0] == '\0') {
        return;
    }
    int ret = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0 && errno != EEXIST) {
        ERR("Unable To Create Directory: %s: %s", path, strerror(errno));
    }
    bool is_dir = false;
    struct stat obj = {};
    ret = stat(path, &obj);
    if (ret == 0) {
        is_dir = S_ISDIR(obj.st_mode);
    }
    if (!is_dir) {
        ERR("Not A Directory: %s", path);
    }
}

// Write To FD
void safe_write(const int fd, const void *buf, const size_t size) {
    if (fd < 0) {
        return;
    }
    const ssize_t bytes_written = write(fd, buf, size);
    if (bytes_written < 0) {
        ERR("Unable To Write Data: %s", strerror(errno));
    }
}

// Get MCPI Home Directory
std::string home_get() {
    const char *home = require_env(_MCPI_HOME_ENV);
    return std::string(home) + std::string(get_home_subdirectory_for_game_data());
}