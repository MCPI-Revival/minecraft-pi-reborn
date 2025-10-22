#include <sys/stat.h>
#include <ranges>
#include <fcntl.h>
#include <ext/stdio_filebuf.h>

#include <LIEF/ELF.hpp>

#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/util/string.h>
#include <libreborn/env/env.h>
#include <libreborn/config.h>
#include <libreborn/log.h>

#include "bootstrap.h"

// Duplicate MCPI Executable Into /tmp
std::string get_patched_exe_path() {
    return get_temp_dir() + reborn_config.app.name;
}
static int create_file() {
    // Lock File
    // Locks are not inheritable on Windows.
#ifndef _WIN32
    const HANDLE lock_fd = lock_file(patched_exe_path.c_str());
    set_and_print_env(_MCPI_LOCK_FD_ENV, safe_to_string(lock_fd).c_str());
#endif
    // Generate New File
    const std::string patched_exe_path = get_patched_exe_path();
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
    // Create New Executable File
    const int fd = create_file();

    // Load Binary
    const std::unique_ptr<LIEF::ELF::Binary> binary = LIEF::ELF::Parser::parse(original_path);

    // Set Interpreter
    binary->interpreter(interpreter);

    // Remove The Existing Needed Libraries
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
    for (const std::string &mod : mods) {
        all_libraries.push_back(translate_native_path_to_linux(mod));
    }
    all_libraries.insert(all_libraries.end(), needed_libraries.begin(), needed_libraries.end());
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

    // Close File
    close(fd);
}

// Linker
std::string get_new_linker(const std::string &binary_directory_linux) {
    std::string linker = linux_path_separator + std::string("lib") + linux_path_separator + "ld-linux-armhf.so.3";
    if (reborn_config.internal.use_prebuilt_armhf_toolchain) {
        linker = binary_directory_linux + linux_path_separator + "sysroot" + linker;
    }
    return linker;
}
std::vector<std::string> get_ld_path(const std::string &binary_directory_linux) {
    // Libraries
    std::vector mcpi_ld_path = {
        std::string("lib") + linux_path_separator + "arm"
    };
    // ARM Sysroot
    if (reborn_config.internal.use_prebuilt_armhf_toolchain) {
        std::vector sysroot_paths = {
            std::string("sysroot") + linux_path_separator + "lib",
            std::string("sysroot") + linux_path_separator + "lib" + linux_path_separator + "arm-linux-gnueabihf",
            std::string("sysroot") + linux_path_separator + "usr" + linux_path_separator + "lib",
            std::string("sysroot") + linux_path_separator + "usr" + linux_path_separator + "lib" + linux_path_separator + "arm-linux-gnueabihf"
        };
        mcpi_ld_path.insert(mcpi_ld_path.end(), sysroot_paths.begin(), sysroot_paths.end());
    }
    // Fix Paths
    for (std::string &path : mcpi_ld_path) {
        path.insert(0, 1, linux_path_separator);
        path.insert(0, binary_directory_linux);
    }
    // Return
    return mcpi_ld_path;
}