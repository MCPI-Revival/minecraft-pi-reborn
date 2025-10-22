#include <string>
#include <vector>

#include <libreborn/log.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "../util/util.h"
#include "bootstrap.h"

// Bootstrap
void bootstrap(const options_t &options) {
    // Debug Information
    print_debug_information();

    // Get Binary Directory
    const std::string binary_directory = get_binary_directory();
    DEBUG("Binary Directory: %s", binary_directory.c_str());

    // Copy SDK
    if (!reborn_is_server()) {
        copy_sdk(binary_directory, false);
    }

    // Resolve Binary Path
    DEBUG("Resolving File Paths...");
    std::string original_game_binary = binary_directory + path_separator + "game" + path_separator + "minecraft-pi";
    original_game_binary = safe_realpath(original_game_binary);
    const char *custom_binary = getenv(MCPI_BINARY_ENV);
    const std::string game_binary = custom_binary ? safe_realpath(custom_binary) : original_game_binary;

    // Configure Preloaded Objects
    DEBUG("Locating Mods...");
    const std::vector<std::string> mcpi_ld_preload = bootstrap_mods(binary_directory);

    // Configure Library Search Path
    DEBUG("Setting Linker Search Paths...");
    const std::string binary_directory_linux = translate_native_path_to_linux(binary_directory);
    const std::vector<std::string> mcpi_ld_path = get_ld_path(binary_directory_linux);

    // Assets
    DEBUG("Finding Assets...");
    bootstrap_assets(original_game_binary);

    // Patch Binary
    DEBUG("Patching ELF...");
    patch_mcpi_elf_dependencies(game_binary, get_new_linker(binary_directory_linux), mcpi_ld_path, mcpi_ld_preload);

    // Fix Environment
    DEBUG("Fixing Environment...");
    set_and_print_env("LD_BIND_NOW", nullptr);
    set_and_print_env("LC_ALL", "C.UTF-8");

    // Start Game
    INFO("Starting Game...");

    // Run
    const std::string runtime_exe = binary_directory + path_separator + "runtime";
    const std::string logger_exe = binary_directory + path_separator + "logger";
    const std::string patched_exe = get_patched_exe_path();
    std::vector<const char *> new_argv = {
        runtime_exe.c_str(),
        patched_exe.c_str(),
        nullptr
    };
    if (!options.disable_logger) {
        new_argv.insert(new_argv.begin(), logger_exe.c_str());
    }
    safe_execvpe(new_argv.data());
}
