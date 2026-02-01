#include "internal.h"

#ifdef _WIN32
#include <fcntl.h>
#else
#include <unistd.h>
#endif

#include <libreborn/log.h>
#include <libreborn/util/logger.h>
#include <libreborn/env/env.h>

// Duplicate A Handle
// This allows multiple loggers to coexist in a single process.
// For instance, when using QEMU with the system-call trampoline,
// the native code and emulated code will run in the same
// process.
static void duplicate_handle(HANDLE &handle) {
#ifdef _WIN32
    const HANDLE current_process = GetCurrentProcess();
    HANDLE out = nullptr;
    if (!DuplicateHandle(current_process, handle, current_process, &out, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        ERR("Unable To Duplicate Handle: %p", handle);
    }
    handle = out;
#else
    const int out = dup(handle);
    if (out < 0) {
        ERR("Unable To Duplicate FD: %i", handle);
    }
    handle = out;
#endif
}

// Disable stdout Buffering
static void disable_buffering(FILE *file) {
    setvbuf(file, nullptr, _IONBF, 0);
}
__attribute__((constructor)) static void disable_stdio_buffering() {
    for (FILE *file : {stdout, stderr}) {
        disable_buffering(file);
    }
}

// Set Log File
// This has three possible states:
// - nullopt: Not Opened Yet
// - nullptr: No Log Attached
// - A Valid Handle: A Log Is Attached
std::optional<FILE *> log_file;
static void open_log_file_from_fd(std::optional<HANDLE> &handle) {
    // Check
    log_file = nullptr;
    if (!handle.has_value()) {
        return;
    }
    // Get FD
    duplicate_handle(handle.value());
    const int fd =
#ifdef _WIN32
        _open_osfhandle(intptr_t(handle.value()), _O_WRONLY | _O_APPEND)
#else
        handle.value()
#endif
        ;
    // Open File
    FILE *file = fdopen(fd, "ab");
    if (file) {
        disable_buffering(file);
        log_file = file;
    }
}

// Retrieve Log File (Open If Needed)
// Data written to this file will be forwarded to the log file.
// This will be retrieved from the process' environment.
FILE *reborn_get_log_file() {
    // Check If File Is Already Opened
    if (log_file.has_value()) {
        return log_file.value();
    }
    // Open Log File
    std::optional<HANDLE> handle;
    const char *env = _MCPI_LOG_FD_ENV;
    if (is_env_set(env)) {
        const char *fd_str = require_env(env);
        handle =
#ifdef _WIN32
            HANDLE(std::stoull(fd_str))
#else
            std::stoi(fd_str)
#endif
            ;
    }
    open_log_file_from_fd(handle);
    // Return
    return reborn_get_log_file();
}

// Close The Log File
__attribute__((destructor)) void close_log_file() {
    FILE *file = log_file.value_or(nullptr);
    if (file) {
        fclose(file);
    }
    log_file = std::nullopt;
}