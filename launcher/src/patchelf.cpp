#include <cstdlib>
#include <sys/stat.h>

#include <LIEF/ELF.hpp>

#include <libreborn/libreborn.h>

#include "patchelf.h"

// Duplicate MCPI Executable Into /tmp
static void duplicate_mcpi_executable(char *new_path) {
    // Ensure Temporary Directory
    {
        // Check If It Exists
        struct stat tmp_stat;
        int exists = stat(MCPI_PATCHED_DIR, &tmp_stat) != 0 ? 0 : S_ISDIR(tmp_stat.st_mode);
        if (!exists) {
            // Doesn't Exist
            if (mkdir(MCPI_PATCHED_DIR, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
                ERR("Unable To Create Temporary Folder: %s", strerror(errno));
            }
        }
    }

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
void patch_mcpi_elf_dependencies(const char *original_path, char *new_path, const char *linker) {
    // Duplicate MCPI executable into /tmp so it can be modified.
    duplicate_mcpi_executable(new_path);

    // Patch File
    {
        std::unique_ptr<LIEF::ELF::Binary> binary = LIEF::ELF::Parser::parse(original_path);
        if (linker != NULL) {
            binary->interpreter(linker);
        }
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

// Get Interpreter
static int dl_iterate_callback(struct dl_phdr_info *info, __attribute__((unused)) size_t size, void *data) {
    // Only Search Current Program
    if (strcmp(info->dlpi_name, "") == 0) {
        for (int i = 0; i < info->dlpi_phnum; i++) {
            if (info->dlpi_phdr[i].p_type == PT_INTERP) {
                // Callback
                *(char **) data = (char *) info->dlpi_phdr[i].p_vaddr;
            }
        }
    }
    return 0;
}
char *patch_get_interpreter() {
    char *interpreter = NULL;
    dl_iterate_phdr(dl_iterate_callback, &interpreter);
    if (interpreter != NULL) {
        interpreter = strdup(interpreter);
    }
    return interpreter;
}
