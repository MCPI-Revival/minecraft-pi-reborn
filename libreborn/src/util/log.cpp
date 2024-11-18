#include <unistd.h>
#include <fcntl.h>
#include <string>

#include <libreborn/log.h>
#include <libreborn/env.h>

// Debug Tag
const char *reborn_debug_tag = "";

// /dev/null FD
static int null_fd = -1;
static void setup_null_fd() {
    if (null_fd == -1) {
        null_fd = open("/dev/null", O_WRONLY | O_APPEND);
    }
}
__attribute__((destructor)) static void close_null_fd() {
    close(null_fd);
}

// Log File
static int log_fd = -1;
int reborn_get_log_fd() {
    if (log_fd >= 0) {
        return log_fd;
    }
    // Open Log File
    const char *fd_str = getenv(_MCPI_LOG_FD_ENV);
    if (fd_str) {
        log_fd = std::stoi(fd_str);
    } else {
        setup_null_fd();
        log_fd = null_fd;
    }
    // Return
    return reborn_get_log_fd();
}
void reborn_set_log(const int fd) {
    // Set Variable
    log_fd = -1;
    set_and_print_env(_MCPI_LOG_FD_ENV, std::to_string(fd).c_str());
}

// Debug Logging
static bool should_print_debug_to_stderr() {
    return getenv(MCPI_DEBUG_ENV) != nullptr;
}
int reborn_get_debug_fd() {
    return should_print_debug_to_stderr() ? STDERR_FILENO : reborn_get_log_fd();
}