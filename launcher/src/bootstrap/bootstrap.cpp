#include <string>
#include <vector>

#include <libreborn/log.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#ifdef MCPI_BUILD_RUNTIME
#include <trampoline/host.h>
extern "C" std::remove_pointer_t<trampoline_t> trampoline;
#endif

#include "../util/util.h"
#include "bootstrap.h"

#define MCPI_BINARY "minecraft-pi"

// Bootstrap
void bootstrap() {
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
    std::string original_game_binary = binary_directory + ("/" MCPI_BINARY);
    original_game_binary = safe_realpath(original_game_binary);
    const char *custom_binary = getenv(MCPI_BINARY_ENV);
    const std::string game_binary = custom_binary ? safe_realpath(custom_binary) : original_game_binary;

    // Configure Preloaded Objects
    DEBUG("Locating Mods...");
    const std::vector<std::string> mcpi_ld_preload = bootstrap_mods(binary_directory);

    // Configure Library Search Path
    DEBUG("Setting Linker Search Paths...");
    const std::vector<std::string> mcpi_ld_path = get_ld_path(binary_directory);

    // Assets
    DEBUG("Finding Assets...");
    bootstrap_assets(original_game_binary);

    // Patch Binary
    DEBUG("Patching ELF...");
    patch_mcpi_elf_dependencies(game_binary, get_new_linker(binary_directory), mcpi_ld_path, mcpi_ld_preload);

    // Fix Environment
    DEBUG("Fixing Environment...");
    set_and_print_env("LD_BIND_NOW", nullptr);
    set_and_print_env("LC_ALL", "C.UTF-8");

    // Start Game
    INFO("Starting Game...");

    // Arguments
    const std::vector args {
#ifdef MCPI_BUILD_RUNTIME
        binary_directory + "/lib/native/runtime",
#endif
        patched_exe_path
    };

    // Run
    static constexpr int new_argc = 1;
    const char *new_argv[new_argc + 1] = {patched_exe_path.c_str(), nullptr};
#ifdef MCPI_BUILD_RUNTIME
    runtime_main(trampoline, new_argc, new_argv);
#else
    safe_execvpe(new_argv, environ);
#endif
}
