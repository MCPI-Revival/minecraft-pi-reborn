#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <libreborn/log.h>
#include <libreborn/exec.h>
#include <libreborn/env.h>

// Debug Tag
const char *reborn_debug_tag = "";

// Log File
static int log_fd = -1;
int reborn_get_log_fd() {
    if (log_fd >= 0) {
        return log_fd;
    }
    // Open Log File
    const char *fd_str = getenv(_MCPI_LOG_FD_ENV);
    log_fd = fd_str ? atoi(fd_str) : open("/dev/null", O_WRONLY | O_APPEND);
    // Check FD
    if (log_fd < 0) {
        ERR("Unable To Open Log: %s", strerror(errno));
    }
    // Return
    return reborn_get_log_fd();
}
void reborn_set_log(const int fd) {
    // Set Variable
    log_fd = -1;
    char buf[128];
    sprintf(buf, "%i", fd);
    set_and_print_env(_MCPI_LOG_FD_ENV, buf);
}

// Debug Logging
static int should_print_debug_to_stderr() {
    return getenv(MCPI_DEBUG_ENV) != NULL;
}
int reborn_get_debug_fd() {
    return should_print_debug_to_stderr() ? STDERR_FILENO : reborn_get_log_fd();
}