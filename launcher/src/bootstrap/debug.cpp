#include <libreborn/log.h>
#include <libreborn/exec.h>
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
    DEBUG("Reborn Version: v%s", MCPI_VERSION);

    // Architecture
    const char *arch =
#ifdef __x86_64__
        "AMD64"
#elif defined(__aarch64__)
        "ARM64"
#elif defined(__arm__)
        "ARM32"
#else
        "Unknown"
#endif
        ;
    DEBUG("Reborn Target Architecture: %s", arch);
}