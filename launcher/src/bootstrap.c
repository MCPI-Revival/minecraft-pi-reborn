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

                        // Check If File Is Accessible
                        int result = access(name, R_OK);
                        if (result == 0) {
                            // Add To LD_PRELOAD
                            string_append(ld_preload, "%s%s", *ld_preload == NULL ? "" : ":", name);
                        } else if (result == -1 && errno != 0) {
                            // Fail
                            WARN("Unable To Access: %s: %s", name, strerror(errno));
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

    // Set Debug Tag
    reborn_debug_tag = "(Launcher) ";

    // Set Default Native Component Environment
#define set_variable_default(name) set_and_print_env("MCPI_NATIVE_" name, getenv(name));
    for_each_special_environmental_variable(set_variable_default);

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
#ifndef MCPI_SERVER_MODE
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
        char *new_path = NULL;
        safe_asprintf(&new_path, "%s/bin", binary_directory);
        // Add Existing PATH
        {
            char *value = getenv("PATH");
            if (value != NULL && strlen(value) > 0) {
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
void run_simple_command(const char *const command[], const char *error) {
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
        linker = patch_get_interpreter();
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
    {
        // Log
        DEBUG("Setting Linker Search Paths...");

        // Prepare
        char *transitive_ld_path = NULL;
        char *mcpi_ld_path = NULL;

        // Library Search Path For Native Components
        {
            // Add Native Library Directory
            safe_asprintf(&transitive_ld_path, "%s/lib/native", binary_directory);

            // Add Host LD_LIBRARY_PATH
            {
                char *value = getenv("LD_LIBRARY_PATH");
                if (value != NULL && strlen(value) > 0) {
                    string_append(&transitive_ld_path, ":%s", value);
                }
            }

            // Set
            set_and_print_env("MCPI_NATIVE_LD_LIBRARY_PATH", transitive_ld_path);
            free(transitive_ld_path);
        }

        // Library Search Path For ARM Components
        {
            // Add ARM Library Directory
            safe_asprintf(&mcpi_ld_path, "%s/lib/arm", binary_directory);

            // Add ARM Sysroot Libraries (Ensure Priority) (Ignore On Actual ARM System)
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
            string_append(&mcpi_ld_path, ":%s/sysroot/lib:%s/sysroot/lib/arm-linux-gnueabihf:%s/sysroot/usr/lib:%s/sysroot/usr/lib/arm-linux-gnueabihf", binary_directory, binary_directory, binary_directory, binary_directory);
#endif

            // Add Host LD_LIBRARY_PATH
            {
                char *value = getenv("LD_LIBRARY_PATH");
                if (value != NULL && strlen(value) > 0) {
                    string_append(&mcpi_ld_path, ":%s", value);
                }
            }

            // Set
            set_and_print_env("MCPI_ARM_LD_LIBRARY_PATH", mcpi_ld_path);
            free(mcpi_ld_path);
        }

        // Setup iconv
        {
            // Native Components
            char *host_gconv_path = getenv("GCONV_PATH");
            set_and_print_env("MCPI_NATIVE_GCONV_PATH", host_gconv_path);

            // ARM Components
#ifdef MCPI_USE_PREBUILT_ARMHF_TOOLCHAIN
            char *gconv_path = NULL;
            safe_asprintf(&gconv_path, "%s/sysroot/usr/lib/gconv", binary_directory);
            set_and_print_env("MCPI_ARM_GCONV_PATH", gconv_path);
            free(gconv_path);
#else
            set_and_print_env("MCPI_ARM_GCONV_PATH", host_gconv_path);
#endif
        }
    }

    // Configure Preloaded Objects
    {
        // Log
        DEBUG("Locating Mods...");

        // Native Components
        char *host_ld_preload = getenv("LD_PRELOAD");
        set_and_print_env("MCPI_NATIVE_LD_PRELOAD", host_ld_preload);

        // ARM Components
        {
            // Prepare
            char *preload = NULL;

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
                char *value = getenv("LD_PRELOAD");
                if (value != NULL && strlen(value) > 0) {
                    string_append(&preload, ":%s", value);
                }
            }

            // Set
            set_and_print_env("MCPI_ARM_LD_PRELOAD", preload);
            free(preload);
        }
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
    new_args[argv_start] = new_mcpi_exe_path;

    // Non-ARM Systems Need QEMU
#ifndef __ARM_ARCH
    argv_start--;
    new_args[argv_start] = QEMU_BINARY;
#endif

    // Setup Environment
    setup_exec_environment(1);

    // Pass LD_* Variables Through QEMU
#ifndef __ARM_ARCH
    char *qemu_set_env = NULL;
#define pass_variable_through_qemu(name) string_append(&qemu_set_env, "%s%s=%s", qemu_set_env == NULL ? "" : ",", name, getenv(name));
    for_each_special_environmental_variable(pass_variable_through_qemu);
    set_and_print_env("QEMU_SET_ENV", qemu_set_env);
    free(qemu_set_env);
    // Treat QEMU Itself As A Native Component
    setup_exec_environment(0);
#endif

    // Run
    const char **new_argv = &new_args[argv_start];
    safe_execvpe(new_argv, (const char *const *) environ);
}
