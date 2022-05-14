#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include <libreborn/libreborn.h>

#include "bootstrap.h"
#include "patchelf.h"

// Duplicate MCPI Executable Into /tmp
#define TMP_DIR "/tmp/.minecraft-pi-patched"
static void duplicate_mcpi_executable() {
    // Get Original Path
    const char *original_path = getenv("MCPI_EXECUTABLE_PATH");

    // Ensure Temporary Directory
    {
        // Check If It Exists
        struct stat tmp_stat;
        int exists = stat(TMP_DIR, &tmp_stat) != 0 ? 0 : S_ISDIR(tmp_stat.st_mode);
        if (!exists) {
            // Doesn't Exist
            if (mkdir(TMP_DIR, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
                ERR("Unable To Create Temporary Folder: %s", strerror(errno));
            }
        }
    }

    // Generate New File
    char new_path[] = TMP_DIR "/XXXXXX";
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
#define patch_mcpi_elf_dependencies_with_extra_patchelf_args(...) \
    ({ \
        const char *const _macro_command[] = { \
            "patchelf", \
            ##__VA_ARGS__, \
            "--remove-needed", "libbcm_host.so", \
            "--remove-needed", "libX11.so.6", \
            "--remove-needed", "libEGL.so", \
            "--replace-needed", "libGLESv2.so", "libGLESv1_CM.so.1", \
            exe, \
            NULL \
        }; \
        int _macro_return_code = 0; \
        char *_macro_output = run_command(_macro_command, &_macro_return_code); \
        if (_macro_output != NULL) { \
            free(_macro_output); \
        } \
        _macro_return_code; \
    })
void patch_mcpi_elf_dependencies(const char *linker) {
    // Duplicate MCPI executable into /tmp so it can be modified.
    duplicate_mcpi_executable();

    // Get Path
    char *exe = getenv("MCPI_EXECUTABLE_PATH");

    // Run patchelf
    int return_code;
    if (linker == NULL) {
        return_code = patch_mcpi_elf_dependencies_with_extra_patchelf_args();
    } else {
        return_code = patch_mcpi_elf_dependencies_with_extra_patchelf_args("--set-interpreter", linker);
    }
    if (return_code != 0) {
        ERR("patchelf Failed: Exit Code: %i", return_code);
    }

    // Fix Permissions
    if (chmod(exe, S_IRUSR | S_IXUSR) != 0) {
        ERR("Unable To Set File Permissions: %s: %s", exe, strerror(errno));
    }
}

// Get Interpreter
char *patch_get_interpreter(const char *file) {
    // Run
    const char *const command[] = {
        "patchelf",
        "--print-interpreter",
        file,
        NULL
    };
    char *output = run_command(command, NULL);
    if (output != NULL) {
        // Trim
        int length = strlen(output);
        if (output[length - 1] == '\n') {
            output[length - 1] = '\0';
        }
    }
    // Return
    return output;
}
