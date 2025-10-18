#include <cstdlib>

#include <libreborn/env/env.h>
#include <libreborn/util/util.h>
#include <libreborn/config.h>

#include "bootstrap/bootstrap.h"
#include "options/parser.h"
#include "logger/logger.h"
#include "util/util.h"
#include "client/configuration.h"
#include "updater/updater.h"

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

    // Setup MCPI_HOME
    setup_home();
    // Create If Needed
    const std::string minecraft_folder = home_get();
    ensure_directory(minecraft_folder.c_str());
}

// Non-Launch Commands
static void handle_non_launch_commands(const options_t &options) {
    // SDK
    if (options.copy_sdk) {
        const std::string binary_directory = get_binary_directory();
        copy_sdk(binary_directory, true);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
    // Updater
    if (options.run_update) {
        Updater *updater = Updater::instance;
        if (updater) {
            updater->run();
            updater->log_status(false);
        } else {
            ERR("Built-In Updater Unavailable, Use System Package Manager");
        }
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
    // Install Desktop File
    if (options.run_install) {
        copy_desktop_file();
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
}

// Start The Game
static void start_game(const options_t &options) {
    // Disable stdout Buffering
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Configure Client Options
    if (!reborn_is_server()) {
        configure_client(options);
    }

    // Start Logging
    if (!options.disable_logger) {
        setup_logger();
    }

    // Bootstrap
    bootstrap();
}

// Main
int main(const int argc, char *argv[]) {
    // Parse Options
    const options_t options = parse_options(argc, argv);

    // Set Debug Tag
    reborn_debug_tag = "(Launcher) ";

    // Debug Logging
    reborn_set_log(-1);
    bind_to_env(MCPI_DEBUG_ENV, options.debug);
    bind_to_env(MCPI_QUIET_ENV, options.quiet);

    // Setup Environment
    setup_environment(options);

    // Handle Non-Launch Commands (Copy SDK, Print Feature Flags, Etc)
    handle_non_launch_commands(options);
    handle_non_launch_client_only_commands(options);

    // Start The Game
    start_game(options);
}