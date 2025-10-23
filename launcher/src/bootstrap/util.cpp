#include <libreborn/log.h>
#include <libreborn/util/exec.h>

#include "bootstrap.h"
#include "libreborn/util/string.h"

// Run Command And Trim The Output
std::string run_command_and_trim(const char *const command[], const char *action) {
    int status = 0;
    const std::vector<unsigned char> *output = run_command(command, &status);
    if (!is_exit_status_success(status)) {
        ERR("Unable To %s", action);
    }
    std::string output_str = (const char *) output->data();
    delete output;
    // Trim
    trim(output_str);
    // Return
    return output_str;
}

// Translate Windows Path To WSL Path
std::string translate_native_path_to_linux(const std::string &path) {
#ifdef _WIN32
    const char *const command[] = {"wsl", "--exec", "wslpath", path.c_str(), nullptr};
    return run_command_and_trim(command, "Translate Native Path To Linux");
#else
    return path;
#endif
}

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
    std::string out = "/tmp";
#endif
    // Add Trailing Path Seperator
    if (!out.ends_with(path_separator)) {
        out += path_separator;
    }
    return out;
}