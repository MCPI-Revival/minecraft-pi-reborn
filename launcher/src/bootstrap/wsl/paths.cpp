#include "../bootstrap.h"

#include <libreborn/util/exec.h>

// Translate Windows Path To WSL Path
std::string translate_native_path_to_linux(const std::string &path) {
    const char *const command[] = {
        "wsl",
        WSL_FLAGS,
        "--exec", "wslpath", "-u", path.c_str(),
        nullptr
    };
    return run_command_and_trim(command, "Translate Native Path To Linux");
}