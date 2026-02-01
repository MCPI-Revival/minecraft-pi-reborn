#include "internal.h"

#include <unistd.h>

#include <libreborn/util/logger.h>
#include <libreborn/env/env.h>
#include <libreborn/util/string.h>

// Initialize Logger Process
// This will set up the process' environment.
static std::optional<pid_t> logger_process;
void reborn_init_log(const std::optional<HANDLE> &fd) {
    // Close Old Log File
    close_log_file();
    // Mark Process As The Logger
    if (log_file.has_value()) {
        logger_process = getpid();
    } else {
        logger_process.reset();
    }
    // Set Variable
    // Future calls of reborn_get_log_file() will retrieve this.
    const std::string env_value = fd.has_value() ? safe_to_string(fd.value()) : "";
    setenv_safe(_MCPI_LOG_FD_ENV, !env_value.empty() ? env_value.c_str() : nullptr);
}

// Check If This Is The Logger
bool is_logger_process() {
    if (logger_process.has_value()) {
        return logger_process.value() == getpid();
    } else {
        return false;
    }
}