#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libreborn/libreborn.h>

#include "bootstrap.h"

// Get All Mods In Folder
static void load(char **ld_preload, const char *folder) {
    int folder_name_length = strlen(folder);
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
    } else if (errno == ENOENT) {
        // Folder Doesn't Exist
    } else {
        // Unable To Open Folder
        ERR("Error Opening Directory: %s: %s", folder, strerror(errno));
    }
}

// Bootstrap Mods
void bootstrap_mods(char *binary_directory) {
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
