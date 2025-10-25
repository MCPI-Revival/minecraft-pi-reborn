#include <string>
#include <vector>
#include <unistd.h>

#include <libreborn/log.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "../util/util.h"
#include "bootstrap.h"

// Bootstrap
int main(MCPI_UNUSED const int argc, MCPI_UNUSED char *argv[]) {
    // Set Debug Tag
    reborn_debug_tag = DEBUG_TAG("Bootstrapper");

    // Debug Information
    print_debug_information();

    // Get Binary Directory
    const std::string binary_directory = get_binary_directory();
    DEBUG("Binary Directory: %s", binary_directory.c_str());

    // Copy SDK
    if (!reborn_is_server()) {
        copy_sdk(binary_directory, false);
    }

    // Download Game (If Needed)
    const std::string original_game_binary = get_game_binary_path();
    if (access(original_game_binary.c_str(), R_OK) != 0) {
        download_game();
    } else {
        DEBUG("Game Already Downloaded");
    }
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
#ifndef _WIN32
    DEBUG("Fixing Environment...");
    set_and_print_env("LD_BIND_NOW", nullptr);
    set_and_print_env("LC_ALL", "C.UTF-8");
#else
    DEBUG("Configuring WSL...");
    configure_wsl();
#endif

    // Start Game
    INFO("Starting Game...");

    // Run
    const std::string patched_exe = get_patched_exe_path();
    constexpr int new_argc = 1;
    const char *new_argv[new_argc + 1] = {
        patched_exe.c_str(),
        nullptr
    };
    start_runtime(new_argc, new_argv);
}
