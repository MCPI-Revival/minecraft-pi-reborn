#include "internal.h"

#include <unistd.h>
#include <cstdarg>

#include <libreborn/log.h>
#include <libreborn/util/logger.h>
#include <libreborn/env/env.h>

// Debug Tag
const char *reborn_debug_tag = "";

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
    if (file != current_log_file && is_logger_process()) {
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
    // Create Format
    std::string format;
    format += log_start;
    format += "Terminated%s";
    format += log_end;
    // Generate String
    static char buf[512];
    snprintf(buf, sizeof(buf), format.c_str(), control_code("0;95"), "CRASH", reason);
    return buf;
}