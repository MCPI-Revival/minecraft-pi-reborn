#include <cstdlib>
#include <libreborn/libreborn.h>
#include <sys/stat.h>

#include "bootstrap.h"
#include "options/parser.h"
#include "crash-report.h"
#include "util.h"
#include "client/configuration.h"

// Bind Options To Environmental Variable
static void bind_to_env(const char *env, const bool value) {
    if (value) {
        set_and_print_env(env, "1");
    }
}
static void setup_environment(const options_t &options) {
    // Clear Internal Variables
    clear_internal_env_vars();

    // Passthrough Options To Game
    bind_to_env(_MCPI_SERVER_MODE_ENV, options.server_mode);
    bind_to_env(_MCPI_BENCHMARK_ENV, options.benchmark);
    bind_to_env(_MCPI_ONLY_GENERATE_ENV, options.only_generate);
    bind_to_env(_MCPI_FORCE_HEADLESS_ENV, options.force_headless);
    bind_to_env(_MCPI_FORCE_NON_HEADLESS_ENV, options.force_non_headless);

    // GTK Dark Mode
    set_and_print_env("GTK_THEME", "Adwaita:dark");

    // Configure PATH
    {
        // Get Binary Directory
        const std::string binary_directory = get_binary_directory();
        std::string new_path = binary_directory + "/bin";
        // Add Existing PATH
        {
            const char *value = getenv("PATH");
            if (value != nullptr && strlen(value) > 0) {
                new_path += std::string(":") + value;
            }
        }
        // Set And Free
        set_and_print_env("PATH", new_path.c_str());
    }

    // Setup MCPI_HOME
    if (!reborn_is_server()) {
        // Ensure $HOME
        const char *home = getenv("HOME");
        if (home == nullptr) {
            ERR("$HOME Is Not Set");
        }
        set_and_print_env(_MCPI_HOME_ENV, home);
    } else {
        // Set Home To Current Directory, So World Data Is Stored There
        char *launch_directory = getcwd(nullptr, 0);
        ALLOC_CHECK(launch_directory);
        set_and_print_env(_MCPI_HOME_ENV, launch_directory);
        free(launch_directory);
    }
    // Create If Needed
    const std::string minecraft_folder = std::string(getenv(_MCPI_HOME_ENV)) + get_home_subdirectory_for_game_data();
    ensure_directory(minecraft_folder.c_str());
}

// Non-Launch Commands
static void handle_non_launch_commands(const options_t &options) {
    if (options.copy_sdk) {
        const std::string binary_directory = get_binary_directory();
        copy_sdk(binary_directory, false);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
}

// Start The Game
static void start_game(const options_t &options) {
    // Disable stdout Buffering
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Setup Crash Reporting
    if (!options.disable_crash_report) {
        setup_crash_report();
    }

    // Configure Client Options
    if (!reborn_is_server()) {
        configure_client(options);
    }

    // Bootstrap
    bootstrap();
}

// Main
int main(int argc, char *argv[]) {
    // Parse Options
    options_t options = parse_options(argc, argv);

    // Set Debug Tag
    reborn_debug_tag = "(Launcher) ";

    // Debug Logging
    unsetenv(_MCPI_LOG_FD_ENV);
    bind_to_env(MCPI_DEBUG_ENV, options.debug);

    // Setup Environment
    setup_environment(options);

    // Handle Non-Launch Commands (Copy SDK, Print Feature Flags, Etc)
    handle_non_launch_commands(options);
    handle_non_launch_client_only_commands(options);

    // Start The Game
    start_game(options);
}