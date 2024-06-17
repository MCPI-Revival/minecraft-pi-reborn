#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <libreborn/util.h>
#include <libreborn/config.h>
#include <libreborn/env.h>

// Safe Version Of pipe()
void safe_pipe2(int pipefd[2], int flags) {
    if (pipe2(pipefd, flags) != 0) {
        ERR("Unable To Create Pipe: %s", strerror(errno));
    }
}

// Check If Two Percentages Are Different Enough To Be Logged
#define SIGNIFICANT_PROGRESS 5
int is_progress_difference_significant(int32_t new_val, int32_t old_val) {
    if (new_val != old_val) {
        if (new_val == -1 || old_val == -1) {
            return 1;
        } else if (new_val == 0 || new_val == 100) {
            return 1;
        } else {
            return new_val - old_val >= SIGNIFICANT_PROGRESS;
        }
    } else {
        return 0;
    }
}

// Lock File
int lock_file(const char *file) {
    int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd == -1) {
        ERR("Unable To Open Lock File: %s: %s", file, strerror(errno));
    }
    if (flock(fd, LOCK_EX) == -1) {
        ERR("Unable To Lock File: %s: %s", file, strerror(errno));
    }
    return fd;
}
void unlock_file(const char *file, int fd) {
    if (flock(fd, LOCK_UN) == -1) {
        ERR("Unable To Unlock File: %s: %s", file, strerror(errno));
    }
    close(fd);
}

// Access Configuration At Runtime
const char *reborn_get_version() {
    return MCPI_VERSION;
}
int reborn_is_headless() {
    static int ret;
    static int is_set = 0;
    if (!is_set) {
        ret = reborn_is_server();
        if (getenv(_MCPI_FORCE_HEADLESS_ENV)) {
            ret = 1;
        } else if (getenv(_MCPI_FORCE_NON_HEADLESS_ENV)) {
            ret = 0;
        }
        is_set = 1;
    }
    return ret;
}
int reborn_is_server() {
    static int ret;
    static int is_set = 0;
    if (!is_set) {
        ret = getenv(_MCPI_SERVER_MODE_ENV) != NULL;
        is_set = 1;
    }
    return ret;
}

// Check $DISPLAY
void reborn_check_display() {
    if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY")) {
        ERR("No display attached! Make sure $DISPLAY or $WAYLAND_DISPLAY is set.");
    }
}

// Home Subdirectory
const char *get_home_subdirectory_for_game_data() {
    if (!reborn_is_server()) {
        // Store Game Data In "~/.minecraft-pi" Instead Of "~/.minecraft" To Avoid Conflicts
        return "/.minecraft-pi";
    } else {
        // Store Game Data In $HOME Root (In Server Mode, $HOME Is Changed To The Launch Directory)
        return "";
    }
}

// Make Sure Directory Exists
void ensure_directory(const char *path) {
    int ret = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0 && errno != EEXIST) {
        ERR("Unable To Create Directory: %s", strerror(errno));
    }
    int is_dir = 0;
    struct stat obj = {};
    ret = stat(path, &obj);
    if (ret == 0) {
        is_dir = S_ISDIR(obj.st_mode);
    }
    if (!is_dir) {
        ERR("Not A Directory: %s", path);
    }
}