#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/config.h>

#include "bootstrap.h"

// Debug Information
static void run_debug_command(const char *const command[], const char *prefix) {
    const std::string output_str = run_command_and_trim(command, "Gather Debug Information");
    // Print
    DEBUG("%s: %s", prefix, output_str.c_str());
}
void print_debug_information() {
    // System Information
    constexpr const char *const command[] = {
#ifndef _WIN32
        "uname", "-a",
#else
        "cmd", "/c", "ver",
#endif
        nullptr
    };
    run_debug_command(command, "System Information");

    // Version
    DEBUG("Reborn Version: %s", reborn_get_fancy_version().c_str());

    // Architecture
    std::string arch = reborn_config.general.arch;
    for (char &c : arch) {
        c = char(std::toupper(c));
    }
    DEBUG("Reborn Target Architecture: %s", arch.c_str());
}