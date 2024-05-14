#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>

#include <libreborn/log.h>
#include <libreborn/exec.h>

// Debug Tag
const char *reborn_debug_tag = "";

// Log File
static int log_fd = -1;
int reborn_get_log_fd() {
    if (log_fd >= 0) {
        return log_fd;
    }
    // Open Log File
    const char *file = getenv(MCPI_LOG_ENV);
    if (file == NULL) {
        file = "/dev/null";
    }
    log_fd = open(file, O_WRONLY | O_APPEND | O_CLOEXEC);
    // Check FD
    if (log_fd < 0) {
        ERR("Unable To Open Log: %s", strerror(errno));
    }
    // Return
    return reborn_get_log_fd();
}
void reborn_lock_log() {
    int ret = flock(reborn_get_log_fd(), LOCK_EX);
    if (ret != 0) {
        ERR("Unable To Lock Log: %s", strerror(errno));
    }
}
void reborn_unlock_log() {
    int ret = flock(reborn_get_log_fd(), LOCK_UN);
    if (ret != 0) {
        ERR("Unable To Unlock Log: %s", strerror(errno));
    }
}
__attribute__((destructor)) void reborn_close_log() {
    if (log_fd >= 0) {
        close(log_fd);
        log_fd = -1;
    }
}
void reborn_set_log(const char *file) {
    // Close Current Log
    reborn_close_log();
    // Set Variable
    set_and_print_env(MCPI_LOG_ENV, file);
}

// Debug Logging
static int should_print_debug_to_stderr() {
    return getenv(MCPI_DEBUG_ENV) != NULL;
}
int reborn_get_debug_fd() {
    return should_print_debug_to_stderr() ? STDERR_FILENO : reborn_get_log_fd();
}
void reborn_lock_debug() {
    if (!should_print_debug_to_stderr()) {
        reborn_lock_log();
    }
}
void reborn_unlock_debug() {
    if (!should_print_debug_to_stderr()) {
        reborn_unlock_log();
    }
}