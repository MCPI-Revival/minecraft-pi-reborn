#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <optional>
#include <cstdarg>

#include <libreborn/log.h>
#include <libreborn/env/env.h>
#include <libreborn/util/string.h>

// Debug Tag
const char *reborn_debug_tag = "";

// Log File
static std::optional<int> log_fd;
int reborn_get_log_fd() {
    if (log_fd.has_value()) {
        return log_fd.value();
    }
    // Open Log File
    const char *fd_str = getenv(_MCPI_LOG_FD_ENV);
    log_fd = fd_str ? std::stoi(fd_str) : -1;
    // Return
    return reborn_get_log_fd();
}
void reborn_set_log(const int fd) {
    // Set Variable
    log_fd = std::nullopt;
    setenv_safe(_MCPI_LOG_FD_ENV, fd >= 0 ? safe_to_string(fd).c_str() : nullptr);
}

// Control Log Levels
static int reborn_get_info_fd() {
    const bool should_print_info_to_stdout = !is_env_set(MCPI_QUIET_ENV);
    return should_print_info_to_stdout ? STDOUT_FILENO : reborn_get_log_fd();
}
int reborn_get_debug_fd() {
    const bool should_print_debug_to_stderr = is_env_set(MCPI_DEBUG_ENV);
    return should_print_debug_to_stderr ? STDERR_FILENO : reborn_get_log_fd();
}

// Logging
#define control_code(x) "\x1b[" x "m"
#define reset control_code("0")
static constexpr const char *log_start = reset "%s[%s]: ";
static constexpr const char *log_end = reset "\n";
static void log(const int fd, const char *color_code, const char *level, const char *format, va_list args) {
    dprintf(fd, log_start, color_code, level);
    vdprintf(fd, format, args);
    dprintf(fd, log_end);
}
void INFO(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(reborn_get_info_fd(), reset, "INFO", format, args);
    va_end(args);
}
void WARN(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(STDERR_FILENO, control_code("0;93"), "WARN", format, args);
    va_end(args);
}
void DEBUG(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(reborn_get_debug_fd(), control_code("0;90"), "DEBUG", format, args);
    va_end(args);
}
void _ERR(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(STDERR_FILENO, control_code("0;91"), "ERR", format, args);
    va_end(args);
    _exit(EXIT_FAILURE);
}

// Crash Message
const char *reborn_get_crash_message(const char *reason) {
    static char buf[512];
    char *begin = buf;
    const char *end = buf + sizeof(buf);
#define write(...) begin += snprintf(begin, end - begin, __VA_ARGS__)
    write(log_start, control_code("0;95"), "CRASH");
    write("Terminated%s", reason);
    write(log_end);
#undef write
    return buf;
}