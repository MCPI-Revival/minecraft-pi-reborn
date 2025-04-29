#include <cstdlib>
#include <sys/stat.h>
#include <ranges>
#include <fcntl.h>
#include <ext/stdio_filebuf.h>

#include <LIEF/ELF.hpp>

#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>

#include "bootstrap.h"

// Duplicate MCPI Executable Into /tmp
const std::string patched_exe_path = std::string("/tmp/") + reborn_config.app.name;
static int create_file() {
    // Lock File
    const int lock_fd = lock_file(patched_exe_path.c_str());
    set_and_print_env(_MCPI_LOCK_FD_ENV, std::to_string(lock_fd).c_str());
    // Generate New File
    unlink(patched_exe_path.c_str());
    const int fd = open(patched_exe_path.c_str(), O_WRONLY | O_CREAT, S_IRWXU);
    if (fd <= 0) {
        ERR("Unable To Open Patched Executable: %s", strerror(errno));
    }
    // Return
    return fd;
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
void patch_mcpi_elf_dependencies(const std::string &original_path, const std::string &interpreter, const std::vector<std::string> &rpath, const std::vector<std::string> &mods) {
    // Duplicate MCPI executable into /tmp so it can be modified.
    const int fd = create_file();

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
    __gnu_cxx::stdio_filebuf<char> buf(dup(fd), std::ios::out);
    std::ostream stream(&buf);
    builder.write(stream);
}

// Linker
std::string get_new_linker(const std::string &binary_directory) {
    std::string linker = "/lib/ld-linux-armhf.so.3";
    if (reborn_config.internal.use_prebuilt_armhf_toolchain) {
        linker = binary_directory + "/sysroot" + linker;
    }
    return linker;
}
std::vector<std::string> get_ld_path(const std::string &binary_directory) {
    // Libraries
    std::vector<std::string> mcpi_ld_path = {
        "lib/arm"
    };
    // ARM Sysroot
    if (reborn_config.internal.use_prebuilt_armhf_toolchain) {
        std::vector<std::string> sysroot_paths = {
            "sysroot/lib",
            "sysroot/lib/arm-linux-gnueabihf",
            "sysroot/usr/lib",
            "sysroot/usr/lib/arm-linux-gnueabihf"
        };
        mcpi_ld_path.insert(mcpi_ld_path.end(), sysroot_paths.begin(), sysroot_paths.end());
    }
    // Fix Paths
    for (std::string &path : mcpi_ld_path) {
        path.insert(0, 1, '/');
        path.insert(0, binary_directory);
    }
    // Return
    return mcpi_ld_path;
}