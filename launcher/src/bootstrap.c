#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include <libreborn/libreborn.h>

#include "bootstrap.h"
#include "patchelf.h"

// Set Environmental Variable
#define PRESERVE_ENVIRONMENTAL_VARIABLE(name) \
    { \
        char *original_env_value = getenv(name); \
        if (original_env_value != NULL) { \
            setenv("ORIGINAL_" name, original_env_value, 1); \
        } \
    }
static void trim(char **value) {
    // Remove Trailing Colon
    int length = strlen(*value);
    if ((*value)[length - 1] == ':') {
        (*value)[length - 1] = '\0';
    }
    if ((*value)[0] == ':') {
        *value = &(*value)[1];
    }
}
void set_and_print_env(const char *name, char *value) {
    // Set Variable With No Trailing Colon
    static const char *unmodified_name_prefix = "MCPI_";
    if (!starts_with(name, unmodified_name_prefix)) {
        trim(&value);
    }

    // Print New Value
    DEBUG("Set %s = %s", name, value);

    // Set The Value
    setenv(name, value, 1);
}
#ifndef __ARM_ARCH
#define PASS_ENVIRONMENTAL_VARIABLE_TO_QEMU(name) \
    { \
        char *old_value = getenv("QEMU_SET_ENV"); \
        char *new_value = NULL; \
        /* Pass Variable */ \
        safe_asprintf(&new_value, "%s%s%s=%s", old_value == NULL ? "" : old_value, old_value == NULL ? "" : ",", name, getenv(name)); \
        setenv("QEMU_SET_ENV", new_value, 1); \
        free(new_value); \
        /* Reset Variable */ \
        RESET_ENVIRONMENTAL_VARIABLE(name); \
    }
#endif

// Get Environmental Variable
static char *get_env_safe(const char *name) {
    // Get Variable Or Blank String If Not Set
    char *ret = getenv(name);
    return ret != NULL ? ret : "";
}

// Get All Mods In Folder
static void load(char **ld_preload, char *folder) {
    int folder_name_length = strlen(folder);
    // Retry Until Successful
    while (1) {
        // Open Folder
        DIR *dp = opendir(folder);
        if (dp != NULL) {
            // Loop Through Folder
            struct dirent *entry = NULL;
            errno = 0;
            while (1) {
                errno = 0;
                entry = readdir(dp);
                if (entry != NULL) {
                    // Check If File Is Regular
                    if (entry->d_type == DT_REG) {
                        // Get Full Name
                        int name_length = strlen(entry->d_name);
                        int total_length = folder_name_length + name_length;
                        char name[total_length + 1];

                        // Concatenate Folder Name And File Name
                        for (int i = 0; i < folder_name_length; i++) {
                            name[i] = folder[i];
                        }
                        for (int i = 0; i < name_length; i++) {
                            name[folder_name_length + i] = entry->d_name[i];
                        }
                        // Add Terminator
                        name[total_length] = '\0';

                        // Check If File Is Executable
                        int result = access(name, R_OK);
                        if (result == 0) {
                            // Add To LD_PRELOAD
                            string_append(ld_preload, ":%s", name);
                        } else if (result == -1 && errno != 0) {
                            // Fail
                            INFO("Unable To Acesss: %s: %s", name, strerror(errno));
                            errno = 0;
                        }
                    }
                } else if (errno != 0) {
                    // Error Reading Contents Of Folder
                    ERR("Error Reading Directory: %s: %s", folder, strerror(errno));
                } else {
                    // Done!
                    break;
                }
            }
            // Close Folder
            closedir(dp);

            // Exit Function
            return;
        } else if (errno == ENOENT) {
            // Folder Doesn't Exists, Attempt Creation
            int ret = mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (ret != 0) {
                // Unable To Create Folder
                ERR("Error Creating Directory: %s: %s", folder, strerror(errno));
            }
            // Continue Retrying
        } else {
            // Unable To Open Folder
            ERR("Error Opening Directory: %s: %s", folder, strerror(errno));
        }
    }
}

#define MCPI_BINARY "minecraft-pi"
#define QEMU_BINARY "qemu-arm"

// Pre-Bootstrap
void pre_bootstrap() {
    // GTK Dark Mode
#ifndef MCPI_HEADLESS_MODE
    set_and_print_env("GTK_THEME", "Adwaita:dark");
#endif

    // AppImage
#ifdef MCPI_IS_APPIMAGE_BUILD
    char *owd = getenv("OWD");
    if (owd != NULL && chdir(owd) != 0) {
        ERR("AppImage: Unable To Fix Current Directory: %s", strerror(errno));
    }
#endif

    // Get Binary Directory
    char *binary_directory = get_binary_directory();

    // Configure PATH
    {
        // Add Library Directory
        char *new_path;
        safe_asprintf(&new_path, "%s/bin", binary_directory);
        // Add Existing PATH
        {
            char *value = get_env_safe("PATH");
            if (strlen(value) > 0) {
                string_append(&new_path, ":%s", value);
            }
        }
        // Set And Free
        set_and_print_env("PATH", new_path);
        free(new_path);
    }

    // Free Binary Directory
    free(binary_directory);
}

// Bootstrap
void bootstrap(int argc, char *argv[]) {
    INFO("Configuring Game...");

    // Get Binary Directory
    char *binary_directory = get_binary_directory();

    // Handle AppImage
    char *usr_prefix = NULL;
#ifdef MCPI_IS_APPIMAGE_BUILD
    usr_prefix = getenv("APPDIR");
#endif
    if (usr_prefix == NULL) {
        usr_prefix = "";
    }

    // Resolve Binary Path & Set MCPI_DIRECTORY
    {
        // Log
        DEBUG("Resolving File Paths...");

        // Resolve Full Binary Path
        char *full_path = NULL;
        safe_asprintf(&full_path, "%s/" MCPI_BINARY, binary_directory);
        char *resolved_path = realpath(full_path, NULL);
        ALLOC_CHECK(resolved_path);
        free(full_path);

        // Set MCPI_EXECUTABLE_PATH
        set_and_print_env("MCPI_EXECUTABLE_PATH", resolved_path);

        // Set MCPI_VANILLA_ASSETS_PATH
        {
            chop_last_component(&resolved_path);
            string_append(&resolved_path, "/data");
            set_and_print_env("MCPI_VANILLA_ASSETS_PATH", resolved_path);
            free(resolved_path);
        }
    }

    // Set MCPI_REBORN_ASSETS_PATH
    {
        char *assets_path = realpath("/proc/self/exe", NULL);
        ALLOC_CHECK(assets_path);
        chop_last_component(&assets_path);
        string_append(&assets_path, "/data");
        set_and_print_env("MCPI_REBORN_ASSETS_PATH", assets_path);
        free(assets_path);
    }

    // Fix MCPI Dependencies
    {
        // Log
        DEBUG("Patching ELF Dependencies...");

        // Find Linker
        char *linker = NULL;
        // Preserve Existing Linker On ARM
#ifndef __arm__
        safe_asprintf(&linker, "%s/usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3", usr_prefix);
#endif

        // Patch
        patch_mcpi_elf_dependencies(linker);

        // Free Linker Path
        if (linker != NULL) {
            free(linker);
        }

        // Verify
        if (!starts_with(getenv("MCPI_EXECUTABLE_PATH"), "/tmp")) {
            IMPOSSIBLE();
        }
    }

    // Configure LD_LIBRARY_PATH
    {
        // Log
        DEBUG("Setting Linker Search Paths...");

        // Preserve
        PRESERVE_ENVIRONMENTAL_VARIABLE("LD_LIBRARY_PATH");
        char *new_ld_path = NULL;

        // Add Library Directory
        safe_asprintf(&new_ld_path, "%s/lib", binary_directory);

        // Load ARM Libraries (Ensure Priority)
        string_append(&new_ld_path, ":%s/usr/lib/arm-linux-gnueabihf:%s/usr/arm-linux-gnueabihf/lib", usr_prefix, usr_prefix);

        // Add LD_LIBRARY_PATH
        {
            char *value = get_env_safe("LD_LIBRARY_PATH");
            if (strlen(value) > 0) {
                string_append(&new_ld_path, ":%s", value);
            }
        }

        // Set And Free
        set_and_print_env("LD_LIBRARY_PATH", new_ld_path);
        free(new_ld_path);
    }

    // Configure LD_PRELOAD
    {
        // Log
        DEBUG("Locating Mods...");

        // Preserve
        PRESERVE_ENVIRONMENTAL_VARIABLE("LD_PRELOAD");
        char *new_ld_preload = NULL;

        // ~/.minecraft-pi/mods
        {
            // Get Mods Folder
            char *mods_folder = NULL;
            safe_asprintf(&mods_folder, "%s" HOME_SUBDIRECTORY_FOR_GAME_DATA "/mods/", getenv("HOME"));
            // Load Mods From ./mods
            load(&new_ld_preload, mods_folder);
            // Free Mods Folder
            free(mods_folder);
        }

        // Built-In Mods
        {
            // Get Mods Folder
            char *mods_folder = NULL;
            safe_asprintf(&mods_folder, "%s/mods/", binary_directory);
            // Load Mods From ./mods
            load(&new_ld_preload, mods_folder);
            // Free Mods Folder
            free(mods_folder);
        }

        // Add LD_PRELOAD
        {
            char *value = get_env_safe("LD_PRELOAD");
            if (strlen(value) > 0) {
                string_append(&new_ld_preload, ":%s", value);
            }
        }

        // Set LD_PRELOAD
        set_and_print_env("LD_PRELOAD", new_ld_preload);
        free(new_ld_preload);
    }

    // Free Binary Directory
    free(binary_directory);

    // Start Game
    INFO("Starting Game...");

    // Arguments
    int argv_start = 1; // argv = &new_args[argv_start]
    const char *new_args[argv_start /* 1 Potential Prefix Argument (QEMU) */ + argc + 1 /* NULL-Terminator */]; //

    // Copy Existing Arguments
    for (int i = 1; i < argc; i++) {
        new_args[i + argv_start] = argv[i];
    }
    // NULL-Terminator
    new_args[argv_start + argc] = NULL;

    // Set Executable Argument
    new_args[argv_start] = getenv("MCPI_EXECUTABLE_PATH");

    // Non-ARM Systems Need QEMU
#ifndef __ARM_ARCH
    argv_start--;
    new_args[argv_start] = QEMU_BINARY;

    // Prevent QEMU From Being Modded
    PASS_ENVIRONMENTAL_VARIABLE_TO_QEMU("LD_LIBRARY_PATH");
    PASS_ENVIRONMENTAL_VARIABLE_TO_QEMU("LD_PRELOAD");
#endif

    // Run
    const char **new_argv = &new_args[argv_start];
    safe_execvpe(new_argv, (const char *const *) environ);
}
