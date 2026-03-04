#include "../util.h"

#include <libreborn/config.h>
#include <libreborn/log.h>
#include <libreborn/util/exec.h>

// Open Dialog
void user_error(const std::string &str) {
    // Throw A Normal Error In Headless Mode
    if (reborn_is_headless()) {
        ERR("%s", str.c_str());
    }

    // Launch Error Dialog
    const std::string binary_directory = get_binary_directory();
    const std::string exe = binary_directory + path_separator + "error";
    const char *const command[] = {exe.c_str(), str.c_str(), nullptr};
    safe_exec(command);
}