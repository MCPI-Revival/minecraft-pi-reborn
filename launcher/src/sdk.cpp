#include <libreborn/libreborn.h>

#include "bootstrap.h"
#include "util.h"

// Log
#define LOG(is_debug, ...) \
    { \
        if (is_debug) { \
            DEBUG(__VA_ARGS__); \
        } else { \
            INFO(__VA_ARGS__); \
        } \
    }

// Copy SDK Into ~/.minecraft-pi
#define HOME_SUBDIRECTORY_FOR_SDK (std::string(get_home_subdirectory_for_game_data()) + "/sdk")
void copy_sdk(const std::string &binary_directory, const bool log_with_debug) {
    // Ensure SDK Directory
    std::string sdk_path;
    {
        sdk_path = std::string(getenv("HOME")) + HOME_SUBDIRECTORY_FOR_SDK;
        const char *const command[] = {"mkdir", "-p", sdk_path.c_str(), nullptr};
        run_simple_command(command, "Unable To Create SDK Directory");
    }

    // Lock File
    std::string lock_file_path = sdk_path + "/.lock";
    int lock_file_fd = lock_file(lock_file_path.c_str());

    // Output Directory
    std::string output = sdk_path + "/" MCPI_SDK_DIR;
    // Source Directory
    std::string source = binary_directory + "/sdk/.";

    // Clean
    {
        const char *const command[] = {"rm", "-rf", output.c_str(), nullptr};
        run_simple_command(command, "Unable To Clean SDK Output Directory");
    }

    // Make Directory
    {
        const char *const command[] = {"mkdir", "-p", output.c_str(), nullptr};
        run_simple_command(command, "Unable To Create SDK Output Directory");
    }

    // Copy
    {
        const char *const command[] = {"cp", "-ar", source.c_str(), output.c_str(), nullptr};
        run_simple_command(command, "Unable To Copy SDK");
    }

    // Log
    LOG(log_with_debug, "Copied SDK To: %s", output.c_str());

    // Unlock File
    unlock_file(lock_file_path.c_str(), lock_file_fd);
}
