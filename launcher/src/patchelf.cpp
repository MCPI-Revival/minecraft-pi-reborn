#include <cstdlib>
#include <sys/stat.h>

#include <LIEF/ELF.hpp>

#include <dlfcn.h>
#include <link.h>

#include <libreborn/libreborn.h>

#include "patchelf.h"

// Duplicate MCPI Executable Into /tmp
static void duplicate_mcpi_executable(char *new_path) {
    // Ensure Temporary Directory
    ensure_directory(MCPI_PATCHED_DIR);

    // Generate New File
    int new_file_fd = mkstemp(new_path);
    if (new_file_fd == -1) {
        ERR("Unable To Create Temporary File: %s", strerror(errno));
    }
    close(new_file_fd);
}

// Fix MCPI Dependencies
static const char *libraries_to_remove[] = {
    "libbcm_host.so",
    "libX11.so.6",
    "libEGL.so",
    "libGLESv2.so",
    "libSDL-1.2.so.0"
};
static const char *libraries_to_add[] = {
    "libmedia-layer-core.so"
};
void patch_mcpi_elf_dependencies(const char *original_path, char *new_path) {
    // Duplicate MCPI executable into /tmp so it can be modified.
    duplicate_mcpi_executable(new_path);

    // Patch File
    {
        std::unique_ptr<LIEF::ELF::Binary> binary = LIEF::ELF::Parser::parse(original_path);
        for (size_t i = 0; i < (sizeof (libraries_to_remove) / sizeof (const char *)); i++) {
            binary->remove_library(libraries_to_remove[i]);
        }
        for (size_t i = 0; i < (sizeof (libraries_to_add) / sizeof (const char *)); i++) {
            binary->add_library(libraries_to_add[i]);
        }
        LIEF::ELF::Builder builder{*binary};
        builder.build();
        builder.write(new_path);
    }

    // Fix Permissions
    if (chmod(new_path, S_IRUSR | S_IXUSR) != 0) {
        ERR("Unable To Set File Permissions: %s: %s", new_path, strerror(errno));
    }
}
