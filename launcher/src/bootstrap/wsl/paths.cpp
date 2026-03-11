#include "../bootstrap.h"

#include <libreborn/util/exec.h>

// Translate Windows Path To WSL Path
std::string translate_native_path_to_linux(const std::string &path) {
    const char *const command[] = {
        "wsl", WSL_FLAGS, "--exec",
        "wslpath", "-u", path.c_str(),
        nullptr
    };
    return run_command_and_trim(command, "Translate Native Path To Linux");
}

// Properly Delete File In Both WSL And Windows
void firm_delete(const std::string &path) {
    // Delete File From WSL
    if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
        const std::string linux_path = translate_native_path_to_linux(path);
        const char *const command[] = {
            "wsl", WSL_FLAGS, "--exec",
            "rm", "-rf", linux_path.c_str(),
            nullptr
        };
        run_command_and_trim(command, "Delete File From WSL");
    }
    // Delete File From Windows
    _unlink(path.c_str());
}