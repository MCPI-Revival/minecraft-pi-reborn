#include <libreborn/log.h>
#include <libreborn/util/exec.h>

#include "util.h"

// Simpler Version Of run_command()
void run_simple_command(const char *const command[], const char *error) {
    int status = 0;
    const std::vector<unsigned char> *output = run_command(command, &status);
    delete output;
    if (!is_exit_status_success(status)) {
        ERR("%s", error);
    }
}

// Chop Off Last Component
void chop_last_component(std::string &str) {
    const std::string::size_type pos = str.find_last_of('/');
    if (pos == std::string::npos) {
        return;
    }
    str = str.substr(0, pos);
}

// Get Binary Directory (Remember To Free)
std::string safe_realpath(const std::string &path) {
    char *raw = realpath(path.c_str(), nullptr);
    if (raw == nullptr) {
        ERR("Unable To Resolve: %s", path.c_str());
    }
    std::string str = raw;
    free(raw);
    return str;
}
std::string get_binary_directory() {
    // Get Path To Current Executable
    std::string exe = safe_realpath("/proc/self/exe");
    // Chop Off Last Component
    chop_last_component(exe);
    // Return
    return exe;
}
