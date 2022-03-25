#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include <libreborn/libreborn.h>

#include "bootstrap.h"
#include "patchelf.h"

// Duplicate MCPI Executable Into /tmp
static void duplicate_mcpi_executable() {
    // Get Original Path
    const char *original_path = getenv("MCPI_EXECUTABLE_PATH");

    // Generate New File
    char new_path[] = "/tmp/.minecraft-pi-XXXXXX";
    int new_file_fd = mkstemp(new_path);
    if (new_file_fd == -1) {
        ERR("Unable To Create Temporary File: %s", strerror(errno));
    }
    FILE *new_file = fdopen(new_file_fd, "wb");
    if (new_file == NULL) {
        ERR("Unable To Open Temporary File: %s", strerror(errno));
    }
    set_and_print_env("MCPI_EXECUTABLE_PATH", new_path);

    // Copy Original File
    {
        // Open Original File
        FILE *original_file = fopen(original_path, "rb");
        if (original_file == NULL) {
            ERR("Unable To Open File: %s", original_path);
        }

        // Copy
#define BUFFER_SIZE 1024
        char buf[BUFFER_SIZE];
        size_t bytes_read = 0;
        while ((bytes_read = fread((void *) buf, 1, BUFFER_SIZE, original_file)) > 0) {
            fwrite((void *) buf, 1, bytes_read, new_file);
            if (ferror(new_file) != 0) {
                ERR("Unable To Write File: %s", new_path);
            }
        }
        if (ferror(original_file) != 0) {
            ERR("Unable To Read File: %s", original_path);
        }

        // Close Original File
        fclose(original_file);
    }

    // Close New File
    fclose(new_file);
    close(new_file_fd);
}

// Fix MCPI Dependencies
void patch_mcpi_elf_dependencies(const char *linker) {
    // Duplicate MCPI executable into /tmp so it can be modified.
    duplicate_mcpi_executable();

    // Get Path
    char *exe = getenv("MCPI_EXECUTABLE_PATH");

    // Run patchelf
    const char *const command[] = {
        "patchelf",
        "--set-interpreter", linker,
        "--remove-needed", "libbcm_host.so",
        "--remove-needed", "libX11.so.6",
        "--remove-needed", "libEGL.so",
        "--replace-needed", "libGLESv2.so", "libGLESv1_CM.so.1",
        exe,
        NULL
    };
    int return_code = 0;
    char *output = run_command(command, &return_code);
    if (output != NULL) {
        free(output);
    }
    if (return_code != 0) {
        ERR("patchelf Failed: Exit Code: %i", return_code);
    }

    // Fix Permissions
    if (chmod(exe, S_IRUSR | S_IXUSR) != 0) {
        ERR("Unable To Set File Permissions: %s: %s", exe, strerror(errno));
    }
}
