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
#include "crash-report.h"

// Set Environmental Variable
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
                            string_append(ld_preload, "%s%s", *ld_preload == NULL ? "" : ":", name);
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

// Exit Handler
static void exit_handler(__attribute__((unused)) int signal_id) {
    // Pass Signal To Child
    murder_children();
    while (wait(NULL) > 0) {}
    _exit(EXIT_SUCCESS);
}

// Pre-Bootstrap
void pre_bootstrap(int argc, char *argv[]) {
    // Disable stdout Buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    // Print Version
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            // Print
            printf("Reborn v%s\n", MCPI_VERSION);
            fflush(stdout);
            exit(EXIT_SUCCESS);
        }
    }

    // GTK Dark Mode
#ifndef MCPI_SERVER_MODE
    set_and_print_env("GTK_THEME", "Adwaita:dark");
#endif

    // Debug Zenity
#ifndef MCPI_HEADLESS_MODE
    {
        const char *is_debug = getenv("MCPI_DEBUG");
        if (is_debug != NULL && strlen(is_debug) > 0) {
            set_and_print_env("ZENITY_DEBUG", "1");
        }
    }
#endif

    // AppImage
#ifdef MCPI_IS_APPIMAGE_BUILD
    {
        char *owd = getenv("OWD");
        if (owd != NULL && chdir(owd) != 0) {
            ERR("AppImage: Unable To Fix Current Directory: %s", strerror(errno));
        }
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

    // Setup Crash Reports
    setup_crash_report();

    // Install Signal Handlers
    struct sigaction act_sigint;
    memset((void *) &act_sigint, 0, sizeof (struct sigaction));
    act_sigint.sa_flags = SA_RESTART;
    act_sigint.sa_handler = &exit_handler;
    sigaction(SIGINT, &act_sigint, NULL);
    struct sigaction act_sigterm;
    memset((void *) &act_sigterm, 0, sizeof (struct sigaction));
    act_sigterm.sa_flags = SA_RESTART;
    act_sigterm.sa_handler = &exit_handler;
    sigaction(SIGTERM, &act_sigterm, NULL);
}

// Copy SDK Into ~/.minecraft-pi
static void run_simple_command(const char *const command[], const char *error) {
    int status = 0;
    char *output = run_command(command, &status);
    if (output != NULL) {
        free(output);
    }
    if (!is_exit_status_success(status)) {
        ERR("%s", error);
    }
}
static void copy_sdk(char *binary_directory) {
    // Output Directory
    char *output = NULL;
    safe_asprintf(&output, "%s" HOME_SUBDIRECTORY_FOR_GAME_DATA "/sdk/" MCPI_SDK_DIR, getenv("HOME"));
    // Source Directory
    char *source = NULL;
    safe_asprintf(&source, "%s/sdk/.", binary_directory);

    // Clean
    {
        const char *const command[] = {"rm", "-rf", output, NULL};
        run_simple_command(command, "Unable To Clean SDK Output Directory");
    }

    // Make Directory
    {
        const char *const command[] = {"mkdir", "-p", output, NULL};
        run_simple_command(command, "Unable To Create SDK Output Directory");
    }

    // Copy
    {
        const char *const command[] = {"cp", "-ar", source, output, NULL};
        run_simple_command(command, "Unable To Copy SDK");
    }

    // Free
    free(output);
    free(source);
}

// Bootstrap
void bootstrap(int argc, char *argv[]) {
    INFO("Configuring Game...");

    // Get Binary Directory
    char *binary_directory = get_binary_directory();

    // Copy SDK
    copy_sdk(binary_directory);

    // Set MCPI_REBORN_ASSETS_PATH
    {
        char *assets_path = realpath("/proc/self/exe", NULL);
        ALLOC_CHECK(assets_path);
        chop_last_component(&assets_path);
        string_append(&assets_path, "/data");
        set_and_print_env("MCPI_REBORN_ASSETS_PATH", assets_path);
        free(assets_path);
    }

    // Resolve Binary Path & Set MCPI_DIRECTORY
    char *resolved_path = NULL;
    {
        // Log
        DEBUG("Resolving File Paths...");

        // Resolve Full Binary Path
        char *full_path = NULL;
        safe_asprintf(&full_path, "%s/" MCPI_BINARY, binary_directory);
        resolved_path = realpath(full_path, NULL);
        ALLOC_CHECK(resolved_path);
        free(full_path);
    }

    // Fix MCPI Dependencies
    char new_mcpi_exe_path[] = MCPI_PATCHED_DIR "/XXXXXX";
    {
        // Log
        DEBUG("Patching ELF Dependencies...");

        // Find Linker
        char *linker = NULL;
        // Select Linker
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
        // Use ARM Sysroot Linker
        safe_asprintf(&linker, "%s/sysroot/lib/ld-linux-armhf.so.3", binary_directory);
#else
        // Use Current Linker
        char *exe = realpath("/proc/self/exe", NULL);
        ALLOC_CHECK(exe);
        linker = patch_get_interpreter(exe);
        free(exe);
#endif

        // Patch
        patch_mcpi_elf_dependencies(resolved_path, new_mcpi_exe_path, linker);

        // Free Linker Path
        if (linker != NULL) {
            free(linker);
        }

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
    char *library_path = NULL;
    {
        // Log
        DEBUG("Setting Linker Search Paths...");

        // Prepare
        char *new_ld_path = NULL;

        // Add Native Library Directory
        safe_asprintf(&new_ld_path, "%s/lib/native", binary_directory);

        // Add LD_LIBRARY_PATH
        {
            char *value = get_env_safe("LD_LIBRARY_PATH");
            if (strlen(value) > 0) {
                string_append(&new_ld_path, ":%s", value);
            }
        }

        // Set LD_LIBRARY_PATH (Used For Everything Except MCPI)
        set_and_print_env("LD_LIBRARY_PATH", new_ld_path);

        // Add ARM Library Directory
        // (This Overrides LD_LIBRARY_PATH Using ld.so's --library-path Option)
        safe_asprintf(&library_path, "%s/lib/arm", binary_directory);

        // Add ARM Sysroot Libraries (Ensure Priority) (Ignore On Actual ARM System)
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
        string_append(&library_path, ":%s/sysroot/lib:%s/sysroot/lib/arm-linux-gnueabihf:%s/sysroot/usr/lib:%s/sysroot/usr/lib/arm-linux-gnueabihf", binary_directory, binary_directory, binary_directory, binary_directory);
#endif

        // Add Remaining LD_LIBRARY_PATH
        string_append(&library_path, ":%s", new_ld_path);

        // Free LD_LIBRARY_PATH
        free(new_ld_path);
    }

    // Configure MCPI's Preloaded Objects
    char *preload = NULL;
    {
        // Log
        DEBUG("Locating Mods...");

        // ~/.minecraft-pi/mods
        {
            // Get Mods Folder
            char *mods_folder = NULL;
            safe_asprintf(&mods_folder, "%s" HOME_SUBDIRECTORY_FOR_GAME_DATA "/mods/", getenv("HOME"));
            // Load Mods From ./mods
            load(&preload, mods_folder);
            // Free Mods Folder
            free(mods_folder);
        }

        // Built-In Mods
        {
            // Get Mods Folder
            char *mods_folder = NULL;
            safe_asprintf(&mods_folder, "%s/mods/", binary_directory);
            // Load Mods From ./mods
            load(&preload, mods_folder);
            // Free Mods Folder
            free(mods_folder);
        }

        // Add LD_PRELOAD
        {
            char *value = get_env_safe("LD_PRELOAD");
            if (strlen(value) > 0) {
                string_append(&preload, ":%s", value);
            }
        }
    }

    // Free Binary Directory
    free(binary_directory);

    // Start Game
    INFO("Starting Game...");

    // Arguments
    int argv_start = 1; // argv = &new_args[argv_start]
    int real_argv_start = argv_start + 5; // ld.so Arguments
    const char *new_args[real_argv_start /* 1 Potential Prefix Argument (QEMU) */ + argc + 1 /* NULL-Terminator */]; //

    // Copy Existing Arguments
    for (int i = 1; i < argc; i++) {
        new_args[i + real_argv_start] = argv[i];
    }
    // NULL-Terminator
    new_args[real_argv_start + argc] = NULL;

    // Set Executable Argument
    new_args[argv_start] = patch_get_interpreter(new_mcpi_exe_path);
    new_args[argv_start + 1] = "--preload";
    new_args[argv_start + 2] = preload;
    new_args[argv_start + 3] = "--library-path";
    new_args[argv_start + 4] = library_path;
    new_args[real_argv_start] = new_mcpi_exe_path;

    // Non-ARM Systems Need QEMU
#ifndef __ARM_ARCH
    argv_start--;
    new_args[argv_start] = QEMU_BINARY;
#endif

    // Run
    const char **new_argv = &new_args[argv_start];
    safe_execvpe(new_argv, (const char *const *) environ);
}
