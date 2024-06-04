#define _FILE_OFFSET_BITS 64

#include <string>
#include <vector>

#include <libreborn/libreborn.h>

#include "util.h"
#include "bootstrap.h"
#include "patchelf.h"

#define MCPI_BINARY "minecraft-pi"
#define QEMU_BINARY "qemu-arm"

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
    const char *arch = "Unknown";
#ifdef __x86_64__
    arch = "AMD64";
#elif defined(__aarch64__)
    arch = "ARM64";
#elif defined(__arm__)
    arch = "ARM32";
#endif
    DEBUG("Reborn Target Architecture: %s", arch);
}

// Bootstrap
void bootstrap() {
    // Debug Information
    print_debug_information();

    // Check Page Size (Not Needed When Using QEMU)
#ifndef MCPI_USE_QEMU
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size != REQUIRED_PAGE_SIZE) {
        ERR("Invalid page size! A page size of %ld bytes is required, but the system size is %ld bytes.", (long) REQUIRED_PAGE_SIZE, page_size);
    }
#endif

    // Get Binary Directory
    char *binary_directory_raw = get_binary_directory();
    const std::string binary_directory = binary_directory_raw;
    free(binary_directory_raw);
    DEBUG("Binary Directory: %s", binary_directory.c_str());

    // Copy SDK
    copy_sdk(binary_directory, true);

    // Set MCPI_REBORN_ASSETS_PATH
    {
        char *assets_path = realpath("/proc/self/exe", nullptr);
        ALLOC_CHECK(assets_path);
        chop_last_component(&assets_path);
        string_append(&assets_path, "/data");
        set_and_print_env("MCPI_REBORN_ASSETS_PATH", assets_path);
        free(assets_path);
    }

    // Resolve Binary Path & Set MCPI_DIRECTORY
    char *resolved_path = nullptr;
    {
        // Log
        DEBUG("Resolving File Paths...");

        // Resolve Full Binary Path
        const std::string full_path = binary_directory + ("/" MCPI_BINARY);
        resolved_path = realpath(full_path.c_str(), nullptr);
        ALLOC_CHECK(resolved_path);
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
        patch_mcpi_elf_dependencies(resolved_path, new_mcpi_exe_path);

        // Verify
        if (!starts_with(new_mcpi_exe_path, MCPI_PATCHED_DIR)) {
            IMPOSSIBLE();
        }
    }

    // Set MCPI_VANILLA_ASSETS_PATH
    {
        char *assets_path = strdup(resolved_path);
        ALLOC_CHECK(assets_path);
        chop_last_component(&assets_path);
        string_append(&assets_path, "/data");
        set_and_print_env("MCPI_VANILLA_ASSETS_PATH", assets_path);
        free(assets_path);
    }

    // Free Resolved Path
    free(resolved_path);

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

            // Add Host LD_LIBRARY_PATH
            {
                char *value = getenv("LD_LIBRARY_PATH");
                if (value != nullptr && strlen(value) > 0) {
                    mcpi_ld_path += value;
                }
            }
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
    // Use Trampoline Host If Needed
#ifdef MCPI_USE_TRAMPOLINE_HOST
    args.push_back("trampoline");
#endif
    // Fix QEMU Bug
#ifdef MCPI_USE_QEMU
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
