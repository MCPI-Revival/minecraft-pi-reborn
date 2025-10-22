#include <fcntl.h>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <sys/file.h>
#endif

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
#define PIPE_ERROR "Unable To Create Pipe"
#ifdef _WIN32
Pipe::Pipe(const bool inheritable_on_windows): read(nullptr), write(nullptr) {
    SECURITY_ATTRIBUTES attr;
    attr.nLength = sizeof(attr);
    attr.bInheritHandle = inheritable_on_windows ? TRUE : FALSE;
    attr.lpSecurityDescriptor = nullptr;
    if (!CreatePipe(const_cast<PHANDLE>(&read), const_cast<PHANDLE>(&write), &attr, 0)) {
        ERR(PIPE_ERROR);
    }
}
#else
Pipe::Pipe(const bool): read(-1), write(-1) {
    int out[2];
    if (pipe(out) != 0) {
        ERR(PIPE_ERROR ": %s", strerror(errno));
    }
    const_cast<int &>(read) = out[0];
    const_cast<int &>(write) = out[1];
}
#endif

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
#define LOCK_FILE_ERROR "Unable To Open Lock File: %s"
#define LOCK_ERROR "Unable To Lock FileL %s"
HANDLE lock_file(const char *file) {
    // Get A New Path
    const std::string lock = std::string(file) + ".lock";
    file = lock.c_str();
    // Lock
#ifdef _WIN32
    const HANDLE fd = CreateFileA(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (fd == INVALID_HANDLE_VALUE) {
        ERR(LOCK_FILE_ERROR, file);
    }
    if (!LockFile(fd, 0, 0, MAXDWORD, MAXDWORD)) {
        ERR(LOCK_ERROR, file);
    }
#else
    const int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0) {
        ERR(LOCK_FILE_ERROR ": %s", file, strerror(errno));
    }
    if (flock(fd, LOCK_EX) == -1) {
        ERR(LOCK_ERROR ": %s", file, strerror(errno));
    }
#endif
    DEBUG("Locked File: %s: " HANDLE_PRINTF, file, fd);
    return fd;
}
#define UNLOCK_ERROR "Unable To Unlock File"
void unlock_file(const HANDLE fd) {
#ifdef _WIN32
    if (!UnlockFile(fd, 0, 0, MAXDWORD, MAXDWORD)) {
        ERR(UNLOCK_ERROR);
    }
#else
    if (flock(fd, LOCK_UN) == -1) {
        ERR(UNLOCK_ERROR ": %i: %s", fd, strerror(errno));
    }
#endif
    CloseHandle(fd);
    DEBUG("Unlocked File: " HANDLE_PRINTF, fd);
}

// Check $DISPLAY
void reborn_check_display() {
#ifndef _WIN32
    if (!is_env_set("DISPLAY") && !is_env_set("WAYLAND_DISPLAY")) {
        ERR("No display attached! Make sure $DISPLAY or $WAYLAND_DISPLAY is set.");
    }
#endif
}

// Home Subdirectory
const char *get_home_subdirectory_for_game_data() {
    if (is_env_set(MCPI_PROFILE_DIRECTORY_ENV)) {
        // No Subdirectory When Using Custom Profile Directory
        return "";
    } else if (!reborn_is_server()) {
        // Store Game Data In "~/.minecraft-pi" Instead Of "~/.minecraft" To Avoid Conflicts
        static std::string str = path_separator + std::string(".minecraft-pi");
        return str.c_str();
    } else {
        // Store Game Data In $HOME Root (In Server Mode, $HOME Is Changed To The Launch Directory)
        return "";
    }
}

// Make Sure Directory Exists
void ensure_directory(const char *path) {
    // Create
    if (path[0] == '\0') {
        return;
    }
    int ret =
#ifdef _WIN32
        _mkdir(path)
#else
        mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif
        ;
    if (ret != 0 && errno != EEXIST) {
        ERR("Unable To Create Directory: %s: %s", path, strerror(errno));
    }
    // Check
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