#include <unistd.h>
#include <fcntl.h>
#include <string>

#include <libreborn/log.h>
#include <libreborn/env/env.h>
#include <libreborn/util/string.h>

// Debug Tag
const char *reborn_debug_tag = "";

// Log File
static constexpr int unset_fd = -1;
static int log_fd = unset_fd;
int reborn_get_log_fd() {
    if (log_fd != unset_fd) {
        return log_fd;
    }
    // Open Log File
    const char *fd_str = getenv(_MCPI_LOG_FD_ENV);
    log_fd = fd_str ? std::stoi(fd_str) : -2;
    // Return
    return reborn_get_log_fd();
}
void reborn_set_log(const int fd) {
    // Set Variable
    log_fd = unset_fd;
    setenv_safe(_MCPI_LOG_FD_ENV, fd >= 0 ? safe_to_string(fd).c_str() : nullptr);
}

// Debug Logging
static bool should_print_debug_to_stderr() {
    return is_env_set(MCPI_DEBUG_ENV);
}
int reborn_get_debug_fd() {
    return should_print_debug_to_stderr() ? STDERR_FILENO : reborn_get_log_fd();
}