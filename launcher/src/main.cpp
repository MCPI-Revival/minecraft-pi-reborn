#include <cstdlib>

#include <libreborn/env/env.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/util/exec.h>
#include <libreborn/config.h>
#include <libreborn/log.h>

#include "options/parser.h"
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
#ifndef _WIN32
    if (options.run_install) {
        copy_desktop_file();
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
#endif
}

// Start The Game
static void start_game(const options_t &options) {
    // Configure Client Options
    if (!reborn_is_server()) {
        configure_client(options);
    }

    // Bootstrap
    const std::string binary_directory = get_binary_directory();
    const std::string bootstrap_exe = binary_directory + path_separator + "bootstrap";
    const std::string logger_exe = binary_directory + path_separator + "logger";
    std::vector<const char *> new_argv = {
        bootstrap_exe.c_str(),
        nullptr
    };
    if (!options.disable_logger) {
        new_argv.insert(new_argv.begin(), logger_exe.c_str());
    }
    safe_execvpe(new_argv.data());
}

// Main
int main(const int argc, char *argv[]) {
    // Set Debug Tag
    reborn_debug_tag = DEBUG_TAG("Launcher");

    // Parse Options
    const options_t options = parse_options(argc, argv);

    // Debug Logging
    reborn_init_log(-1);
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