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
#define HOME_SUBDIRECTORY_FOR_SDK HOME_SUBDIRECTORY_FOR_GAME_DATA "/sdk"
void copy_sdk(char *binary_directory, int log_with_debug) {
    // Ensure SDK Directory
    {
        char *sdk_path = NULL;
        safe_asprintf(&sdk_path, "%s" HOME_SUBDIRECTORY_FOR_SDK, getenv("HOME"));
        const char *const command[] = {"mkdir", "-p", sdk_path, NULL};
        run_simple_command(command, "Unable To Create SDK Directory");
    }

    // Lock File
    char *lock_file_path = NULL;
    safe_asprintf(&lock_file_path, "%s" HOME_SUBDIRECTORY_FOR_SDK "/.lock", getenv("HOME"));
    int lock_file_fd = lock_file(lock_file_path);

    // Output Directory
    char *output = NULL;
    safe_asprintf(&output, "%s" HOME_SUBDIRECTORY_FOR_SDK "/" MCPI_SDK_DIR, getenv("HOME"));
    // Source Directory
    char *source = NULL;
    safe_asprintf(&source, "%s/sdk/.", binary_directory);

    // Clean
    {
        const char *const command[] = {"rm", "-rf", output, NULL};
        run_simple_command(command, "Unable To Clean SDK Output Directory");
    }

    // Make Directory
    {
        const char *const command[] = {"mkdir", "-p", output, NULL};
        run_simple_command(command, "Unable To Create SDK Output Directory");
    }

    // Copy
    {
        const char *const command[] = {"cp", "-ar", source, output, NULL};
        run_simple_command(command, "Unable To Copy SDK");
    }

    // Log
    LOG(log_with_debug, "Copied SDK To: %s", output);

    // Free
    free(output);
    free(source);

    // Unlock File
    unlock_file(lock_file_path, lock_file_fd);
    free(lock_file_path);
}
