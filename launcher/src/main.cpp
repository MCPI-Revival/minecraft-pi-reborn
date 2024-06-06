#include <cstdlib>
#include <libreborn/libreborn.h>
#include <sys/stat.h>

#ifdef MCPI_USE_NATIVE_TRAMPOLINE
#include <trampoline/types.h>
#endif

#include "bootstrap.h"
#include "options/parser.h"
#include "crash-report.h"
#include "util.h"
#ifndef MCPI_SERVER_MODE
#include "client/configuration.h"
#endif

// Bind Options To Environmental Variable
static void bind_to_env(const char *env, const bool value) {
    const bool force = env[0] == '_';
    if (force || value) {
        set_and_print_env(env, value ? "1" : nullptr);
    }
}
static void setup_environment(const options_t &options) {
    // Passthrough Options To Game
#ifndef MCPI_SERVER_MODE
    bind_to_env("_MCPI_BENCHMARK", options.benchmark);
#else
    bind_to_env("_MCPI_ONLY_GENERATE", options.only_generate);
#endif
#ifdef MCPI_USE_NATIVE_TRAMPOLINE
    bind_to_env(TRAMPOLINE_USE_PTRACE_ENV, options.use_ptrace_trampoline);
#endif

    // GTK Dark Mode
#ifndef MCPI_HEADLESS_MODE
    set_and_print_env("GTK_THEME", "Adwaita:dark");
#endif

    // Configure PATH
    {
        // Get Binary Directory
        char *binary_directory = get_binary_directory();
        std::string new_path = std::string(binary_directory) + "/bin";
        free(binary_directory);
        // Add Existing PATH
        {
            char *value = getenv("PATH");
            if (value != nullptr && strlen(value) > 0) {
                new_path += std::string(":") + value;
            }
        }
        // Set And Free
        set_and_print_env("PATH", new_path.c_str());
    }
}

// Non-Launch Commands
static void handle_non_launch_commands(const options_t &options) {
    if (options.copy_sdk) {
        char *binary_directory = get_binary_directory();
        copy_sdk(binary_directory, false);
        free(binary_directory);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
}

// Exit Handler
static void exit_handler(__attribute__((unused)) int signal_id) {
    // Pass Signal To Child
    murder_children();
    while (wait(nullptr) > 0) {}
    _exit(EXIT_SUCCESS);
}

// Start The Game
static void start_game(const options_t &options) {
    // Disable stdout Buffering
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Environemntal Variable Options
    setup_environment(options);

    // Setup Crash Reporting
    if (!options.disable_crash_report) {
        setup_log_file();
        setup_crash_report();
    }

    // Install Signal Handlers
    struct sigaction act_sigint = {};
    act_sigint.sa_flags = SA_RESTART;
    act_sigint.sa_handler = &exit_handler;
    sigaction(SIGINT, &act_sigint, nullptr);
    struct sigaction act_sigterm = {};
    act_sigterm.sa_flags = SA_RESTART;
    act_sigterm.sa_handler = &exit_handler;
    sigaction(SIGTERM, &act_sigterm, nullptr);

    // Setup Home
#ifndef MCPI_SERVER_MODE
    // Ensure $HOME
    const char *home = getenv("HOME");
    if (home == nullptr) {
        ERR("$HOME Isn't Set");
    }
    // Create If Needed
    {
        std::string minecraft_folder = std::string(home) + HOME_SUBDIRECTORY_FOR_GAME_DATA;
        struct stat tmp_stat = {};
        bool exists = stat(minecraft_folder.c_str(), &tmp_stat) != 0 ? false : S_ISDIR(tmp_stat.st_mode);
        if (!exists) {
            // Doesn't Exist
            if (mkdir(minecraft_folder.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
                ERR("Unable To Create Data Directory: %s", strerror(errno));
            }
        }
    }
#else
    // Set Home To Current Directory, So World Data Is Stored There
    char *launch_directory = getcwd(NULL, 0);
    set_and_print_env("HOME", launch_directory);
    free(launch_directory);
#endif

    // Configure Client Options
#ifndef MCPI_SERVER_MODE
    configure_client(options);
#endif

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
    unsetenv(MCPI_LOG_ENV);
    bind_to_env(MCPI_DEBUG_ENV, options.debug);

    // Handle Non-Launch Commands (Copy SDK, Print Feature Flags, Etc)
    handle_non_launch_commands(options);
#ifndef MCPI_SERVER_MODE
    handle_non_launch_client_only_commands(options);
#endif

    // Check Environment
#ifndef MCPI_SERVER_MODE
    // Code After This Can Safely Open A Window
    check_environment_client();
#endif

    // Start The Game
    start_game(options);
}