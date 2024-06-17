#define _FILE_OFFSET_BITS 64

#include <string>
#include <vector>

#include <libreborn/libreborn.h>

#include "util.h"
#include "bootstrap.h"
#include "patchelf.h"

#define MCPI_BINARY "minecraft-pi"

#define REQUIRED_PAGE_SIZE 4096

// Debug Information
static void run_debug_command(const char *const command[], const char *prefix) {
    int status = 0;
    char *output = run_command(command, &status, nullptr);
    if (output != nullptr) {
        // Remove Newline
        size_t length = strlen(output);
        if (length > 0 && output[length - 1] == '\n') {
            output[length - 1] = '\0';
        }

        // Print
        DEBUG("%s: %s", prefix, output);
        free(output);
    }
    if (!is_exit_status_success(status)) {
        ERR("Unable To Gather Debug Information");
    }
}
static void print_debug_information() {
    // System Information
    const char *const command[] = {"uname", "-a", nullptr};
    run_debug_command(command, "System Information");

    // Version
    DEBUG("Reborn Version: v%s", MCPI_VERSION);

    // Architecture
    const char *arch =
#ifdef __x86_64__
        "AMD64"
#elif defined(__aarch64__)
        "ARM64"
#elif defined(__arm__)
        "ARM32"
#else
        "Unknown"
#endif
        ;
    DEBUG("Reborn Target Architecture: %s", arch);
}

// Bootstrap
void bootstrap() {
    // Debug Information
    print_debug_information();

    // Check Page Size (Not Needed When Using QEMU)
#ifndef MCPI_RUNTIME_IS_QEMU
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size != REQUIRED_PAGE_SIZE) {
        ERR("Invalid page size! A page size of %ld bytes is required, but the system size is %ld bytes.", (long) REQUIRED_PAGE_SIZE, page_size);
    }
#endif

    // Get Binary Directory
    const std::string binary_directory = get_binary_directory();
    DEBUG("Binary Directory: %s", binary_directory.c_str());

    // Copy SDK
    if (!reborn_is_server()) {
        copy_sdk(binary_directory, true);
    }

    // Set MCPI_REBORN_ASSETS_PATH
    {
        std::string assets_path = safe_realpath("/proc/self/exe");
        chop_last_component(assets_path);
        assets_path += "/data";
        set_and_print_env(_MCPI_REBORN_ASSETS_PATH_ENV, assets_path.c_str());
    }

    // Resolve Binary Path & Set MCPI_DIRECTORY
    std::string game_binary;
    {
        // Log
        DEBUG("Resolving File Paths...");

        // Resolve Full Binary Path
        const std::string full_path = binary_directory + ("/" MCPI_BINARY);
        game_binary = safe_realpath(full_path);
    }

    // Fix MCPI Dependencies
    char new_mcpi_exe_path[] = MCPI_PATCHED_DIR "/XXXXXX";
    std::string linker;
    {
        // Log
        DEBUG("Patching ELF Dependencies...");

        // Find Linker
        linker = "/lib/ld-linux-armhf.so.3";
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
        // Use ARM Sysroot Linker
        linker = binary_directory + "/sysroot" + linker;
#endif

        // Patch
        patch_mcpi_elf_dependencies(game_binary.c_str(), new_mcpi_exe_path);

        // Verify
        if (!starts_with(new_mcpi_exe_path, MCPI_PATCHED_DIR)) {
            IMPOSSIBLE();
        }
    }

    // Set MCPI_VANILLA_ASSETS_PATH
    {
        std::string assets_path = game_binary;
        chop_last_component(assets_path);
        assets_path += "/data";
        set_and_print_env(_MCPI_VANILLA_ASSETS_PATH_ENV, assets_path.c_str());
    }

    // Configure Library Search Path
    std::string mcpi_ld_path = "";
    {
        // Log
        DEBUG("Setting Linker Search Paths...");

        // Library Search Path For ARM Components
        {
            // Add ARM Library Directory
            mcpi_ld_path += binary_directory + "/lib/arm:";

            // Add ARM Sysroot Libraries (Ensure Priority) (Ignore On Actual ARM System)
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
            mcpi_ld_path += binary_directory + "/sysroot/lib:";
            mcpi_ld_path += binary_directory + "/sysroot/lib/arm-linux-gnueabihf:";
            mcpi_ld_path += binary_directory + "/sysroot/usr/lib:";
            mcpi_ld_path += binary_directory + "/sysroot/usr/lib/arm-linux-gnueabihf:";
#endif
        }
    }

    // Configure Preloaded Objects
    std::string mcpi_ld_preload;
    {
        // Log
        DEBUG("Locating Mods...");

        // ARM Components
        mcpi_ld_preload = bootstrap_mods(binary_directory);
    }

    // Start Game
    INFO("Starting Game...");

    // Arguments
    std::vector<std::string> args;
    // Use Extra If Needed
#ifdef MCPI_BUILD_RUNTIME
    args.push_back("runtime");
#endif
    // Fix QEMU Bug
#ifdef MCPI_RUNTIME_IS_QEMU
    args.push_back("-B");
    args.push_back("0x40000"); // Arbitrary Value (Aligns To 4k And 16k Page Sizes)
#endif

    // Setup Linker
    args.push_back(linker);
    args.push_back("--library-path");
    args.push_back(mcpi_ld_path);
    args.push_back("--preload");
    args.push_back(mcpi_ld_preload);

    // Specify MCPI Binary
    args.push_back(new_mcpi_exe_path);

    // Run
    const char *new_argv[args.size() + 1];
    for (std::vector<std::string>::size_type i = 0; i < args.size(); i++) {
        new_argv[i] = args[i].c_str();
    }
    new_argv[args.size()] = nullptr;
    safe_execvpe(new_argv, environ);
}
