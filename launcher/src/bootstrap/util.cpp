#ifndef _WIN32
#include <cstring>
#include <unistd.h>

#include <libreborn/env/env.h>
#endif

#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/util/string.h>

#include "bootstrap.h"

// Run Command And Trim The Output
std::string run_command_and_trim(const char *const command[], const char *action) {
    const CommandResult result = run_command(command);
    if (!is_exit_status_success(result.status)) {
        ERR("Unable To %s", action);
    }
    std::string output_str = result.str<std::string>();
    // Trim
    trim(output_str);
    // Return
    return output_str;
}

// Unnecessary Functions On Plain Linux
#ifndef _WIN32
std::string translate_native_path_to_linux(const std::string &path) {
    return path;
}
void firm_delete(const std::string &path) {
    unlink(path.c_str());
}
#endif

// Get Temporary Directory
std::string get_temp_dir() {
#ifdef _WIN32
    constexpr DWORD max_length = PATH_MAX + 1;
    char path[max_length];
    const DWORD ret = GetTempPathA(max_length, path);
    if (ret == 0) {
        ERR("Unable To Get Temporary Directory");
    }
    std::string out = path;
#else
    const std::optional<std::string> env = getenv_safe("TMPDIR");
    std::string out = env.value_or(P_tmpdir);
#endif
    // Add Trailing Path Seperator
    if (!out.ends_with(path_separator)) {
        out += path_separator;
    }
    return out;
}