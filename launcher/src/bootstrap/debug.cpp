#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/config.h>

#include "bootstrap.h"

// Debug Information
static void run_debug_command(const char *const command[], const char *prefix) {
    int status = 0;
    const std::vector<unsigned char> *output = run_command(command, &status);
    if (!is_exit_status_success(status)) {
        ERR("Unable To Gather Debug Information");
    }
    std::string output_str = (const char *) output->data();
    delete output;
    // Trim
    const std::string::size_type length = output_str.length();
    if (length > 0 && output_str[length - 1] == '\n') {
        output_str.pop_back();
    }
    // Print
    DEBUG("%s: %s", prefix, output_str.c_str());
}
void print_debug_information() {
    // System Information
    constexpr const char *const command[] = {"uname", "-a", nullptr};
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