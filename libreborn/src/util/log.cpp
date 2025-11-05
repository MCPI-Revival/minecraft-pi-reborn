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

// Set Log File
static std::optional<FILE *> log_file;
static void open_log_file_from_fd(const int fd) {
    log_file = nullptr;
    if (fd >= 0) {
        FILE *file = fdopen(fd, "ab");
        if (file) {
            log_file = file;
        }
    }
}
FILE *reborn_get_log_file() {
    if (log_file.has_value()) {
        return log_file.value();
    }
    // Open Log File
    const char *fd_str =
#ifndef _WIN32
        getenv(_MCPI_LOG_FD_ENV)
#else
        nullptr
#endif
        ;
    if (fd_str) {
        int fd = std::stoi(fd_str);
        fd = dup(fd); // Prevent the real FD from being closed.
        open_log_file_from_fd(fd);
    } else {
        log_file = nullptr;
    }
    // Return
    return reborn_get_log_file();
}
__attribute__((destructor)) static void close_log_file() {
    FILE *file = log_file.value_or(nullptr);
    if (file) {
        fclose(file);
    }
    log_file = std::nullopt;
}
static pid_t logger_process = -1;
void reborn_init_log(const int fd) {
    // Configure Local State
    close_log_file();
    open_log_file_from_fd(fd);
    if (log_file.value_or(nullptr)) {
        logger_process = getpid();
    }
    // Set Variable
    const std::string env_value
#ifndef _WIN32
        = fd >= 0 ? safe_to_string(fd) : ""
#endif
        ;
    setenv_safe(_MCPI_LOG_FD_ENV, !env_value.empty() ? env_value.c_str() : nullptr);
}

// Control Log Levels
static FILE *reborn_get_info_file() {
    const bool should_print_info_to_stdout = !is_env_set(MCPI_QUIET_ENV);
    return should_print_info_to_stdout ? stdout : reborn_get_log_file();
}
FILE *reborn_get_debug_file() {
    const bool should_print_debug_to_stderr = is_env_set(MCPI_DEBUG_ENV);
    return should_print_debug_to_stderr ? stderr : reborn_get_log_file();
}

// Logging
#define control_code(x) "\x1b[" x "m"
#define reset control_code("0")
static constexpr const char *log_start = reset "%s[%s]: ";
static constexpr const char *log_end = reset "\n";
static void log(FILE *file, const char *color_code, const char *level, const char *format, va_list args) {
    // The logger process's output is not recorded,
    // so write everything to the log manually.
    FILE *current_log_file = reborn_get_log_file();
    const bool is_logger_process = logger_process == getpid();
    if (file != current_log_file && is_logger_process) {
        va_list args_copy;
        va_copy(args_copy, args);
        log(current_log_file, color_code, level, format, args_copy);
        va_end(args_copy);
    }

    // Check File
    if (!file) {
        return;
    }
    // Write
    fprintf(file, log_start, color_code, level);
    vfprintf(file, format, args);
    fprintf(file, log_end);
    fflush(file);
}
void INFO(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(reborn_get_info_file(), reset, "INFO", format, args);
    va_end(args);
}
void WARN(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(stderr, control_code("0;93"), "WARN", format, args);
    va_end(args);
}
void _DEBUG(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(reborn_get_debug_file(), control_code("0;90"), "DEBUG", format, args);
    va_end(args);
}
void _ERR(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log(stderr, control_code("0;91"), "ERR", format, args);
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

// Disable stdout Buffering
__attribute__((constructor)) static void disable_stdout_buffering() {
    setvbuf(stdout, nullptr, _IONBF, 0);
}