#include <string>
#include <vector>

#include <libreborn/log.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>
#include <libreborn/util/exec.h>

#include "../util/util.h"
#include "bootstrap.h"

#define MCPI_BINARY "minecraft-pi"

#define REQUIRED_PAGE_SIZE 4096

// Bootstrap
void bootstrap(const options_t &options) {
    // Debug Information
    print_debug_information();

    // Check Page Size
    const long page_size = sysconf(_SC_PAGESIZE);
    if (page_size != REQUIRED_PAGE_SIZE) {
        CONDITIONAL_ERR(!options.skip_pagesize_check, "Invalid page size! A page size of %ld bytes is required, but the system size is %ld bytes.", (long) REQUIRED_PAGE_SIZE, page_size);
    }

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
    char new_mcpi_exe_path[] = MCPI_PATCHED_DIR "/XXXXXX";
    DEBUG("Patching ELF...");
    patch_mcpi_elf_dependencies(game_binary, new_mcpi_exe_path, get_new_linker(binary_directory), mcpi_ld_path, mcpi_ld_preload);

    // Start Game
    INFO("Starting Game...");

    // Arguments
    const std::vector<std::string> args {
#ifdef MCPI_BUILD_RUNTIME
        "runtime",
#endif
        new_mcpi_exe_path
    };

    // Run
    const char *new_argv[args.size() + 1];
    for (std::vector<std::string>::size_type i = 0; i < args.size(); i++) {
        new_argv[i] = args[i].c_str();
    }
    new_argv[args.size()] = nullptr;
    safe_execvpe(new_argv, environ);
}
