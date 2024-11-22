#include <cstdlib>
#include <sys/stat.h>
#include <ranges>

#include <LIEF/ELF.hpp>

#include <libreborn/util.h>
#include <libreborn/config.h>

#include "bootstrap.h"

// Duplicate MCPI Executable Into /tmp
static void duplicate_mcpi_executable(char *new_path) {
    // Ensure Temporary Directory
    ensure_directory(MCPI_PATCHED_DIR);

    // Generate New File
    const int new_file_fd = mkstemp(new_path);
    if (new_file_fd == -1) {
        ERR("Unable To Create Temporary File: %s", strerror(errno));
    }
    close(new_file_fd);
}

// Fix MCPI Dependencies
static std::vector<std::string> needed_libraries = {
    "libmedia-layer-core.so",
    "libpng12.so.0",
    "libstdc++.so.6",
    "libm.so.6",
    "libgcc_s.so.1",
    "libc.so.6",
    "libpthread.so.0"
};
static std::vector<std::string> function_prefixes_to_patch = {
    "SDL_",
    "gl"
};
void patch_mcpi_elf_dependencies(const std::string &original_path, char *new_path, const std::string &interpreter, const std::vector<std::string> &rpath, const std::vector<std::string> &mods) {
    // Duplicate MCPI executable into /tmp so it can be modified.
    duplicate_mcpi_executable(new_path);

    // Load Binary
    const std::unique_ptr<LIEF::ELF::Binary> binary = LIEF::ELF::Parser::parse(original_path);

    // Set Interpreter
    binary->interpreter(interpreter);

    // Remove Existing Needed Libraries
    std::vector<std::string> to_remove;
    for (const LIEF::ELF::DynamicEntry &entry : binary->dynamic_entries()) {
        const LIEF::ELF::DynamicEntryLibrary *library = dynamic_cast<const LIEF::ELF::DynamicEntryLibrary *>(&entry);
        if (library) {
            to_remove.push_back(library->name());
        }
    }
    for (const std::string &library : to_remove) {
        binary->remove_library(library);
    }

    // Setup RPath
    binary->add(LIEF::ELF::DynamicEntryRpath(rpath));

    // Add Libraries
    std::vector<std::string> all_libraries;
    for (const std::vector<std::string> &list : {mods, needed_libraries}) {
        all_libraries.insert(all_libraries.end(), list.begin(), list.end());
    }
    for (const std::string &library : all_libraries | std::views::reverse) {
        binary->add_library(library);
    }

    // Fix Symbol Names
    for (LIEF::ELF::Symbol &symbol : binary->dynamic_symbols()) {
        if (symbol.is_function()) {
            for (const std::string &prefix : function_prefixes_to_patch) {
                if (symbol.name().rfind(prefix, 0) == 0) {
                    symbol.name("media_" + symbol.name());
                    break;
                }
            }
        }
    }

    // Write Binary
    LIEF::ELF::Builder builder{*binary};
    builder.build();
    builder.write(new_path);

    // Fix Permissions
    if (chmod(new_path, S_IRUSR | S_IXUSR) != 0) {
        ERR("Unable To Set File Permissions: %s: %s", new_path, strerror(errno));
    }
}

// Linker
std::string get_new_linker(const std::string &binary_directory) {
    std::string linker = "/lib/ld-linux-armhf.so.3";
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
    linker = binary_directory + "/sysroot" + linker;
#else
    (void) binary_directory;
#endif
    return linker;
}
std::vector<std::string> get_ld_path(const std::string &binary_directory) {
    std::vector<std::string> mcpi_ld_path = {
        // ARM Sysroot
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
        "sysroot/lib",
        "sysroot/lib/arm-linux-gnueabihf",
        "sysroot/usr/lib",
        "sysroot/usr/lib/arm-linux-gnueabihf",
#endif
        // Libraries
        "lib/arm"
    };
    // Fix Paths
    for (std::string &path : mcpi_ld_path) {
        path.insert(0, 1, '/');
        path.insert(0, binary_directory);
    }
    // Return
    return mcpi_ld_path;
}