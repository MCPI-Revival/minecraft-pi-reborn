#define _FILE_OFFSET_BITS 64 
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

static int starts_with(const char *s, const char *t) {
    return strncmp(s, t, strlen(t)) == 0;
}

static int ends_with(const char *s, const char *t) {
    size_t slen = strlen(s);
    size_t tlen = strlen(t);
    if (tlen > slen) return 1;
    return strcmp(s + slen - tlen, t) == 0;
}

static void trim(char *value) {
    // Remove Trailing Colon
    int length = strlen(value);
    if (value[length - 1] == ':') {
        value[length - 1] = '\0';
    }
}

static void set_and_print_env(char *name, char *value) {
    // Set Variable With Not Trailing Colon
    trim(value);

    fprintf(stderr, "Set %s = %s\n", name, value);
    setenv(name, value, 1);
}

static char *get_env_safe(const char *name) {
    // Get Variable Or Blank String If Not Set
    char *ret = getenv(name);
    return ret != NULL ? ret : "";
}

static void load(char **ld_path, char **ld_preload, char *folder) {
    int folder_name_length = strlen(folder);
    while (1) {
        DIR *dp = opendir(folder);
        if (dp != NULL) {
            struct dirent *entry = NULL;
            errno = 0;
            while (1) {
                entry = readdir(dp);
                if (entry != NULL) {
                    // Check If File Is A Shared Library
                    if (entry->d_type == DT_REG && starts_with(entry->d_name, "lib") && ends_with(entry->d_name, ".so")) {
                        int name_length = strlen(entry->d_name);
                        int total_length = folder_name_length + name_length;
                        char name[total_length + 1];

                        for (int i = 0; i < folder_name_length; i++) {
                            name[i] = folder[i];
                        }
                        for (int i = 0; i < name_length; i++) {
                            name[folder_name_length + i] = entry->d_name[i];
                        }

                        name[total_length] = '\0';

                        // Add To LD_PRELOAD
                        asprintf(ld_preload, "%s:%s", name, *ld_preload);
                    }
                } else if (errno != 0) {
                    // Error Reading Contents Of Folder
                    fprintf(stderr, "Error Reading Directory: %s\n", strerror(errno));
                    exit(1);
                } else {
                    break;
                }
            }
            closedir(dp);

            // Add To LD_LIBRARY_PATH
            asprintf(ld_path, "%s:%s", *ld_path, folder);

            return;
        } else if (errno == ENOENT) {
            // Folder Doesn't Exists, Attempt Creation
            char *cmd = NULL;
            asprintf(&cmd, "mkdir -p %s", folder);
            int ret = system(cmd);
            if (ret != 0) {
                exit(ret);
            }
        } else {
            // Unable To Open Folder
            fprintf(stderr, "Error Opening Directory: %s\n", strerror(errno));
            exit(1);
        }
    }
}

int main(__attribute__((unused)) int argc, char *argv[]) {
    fprintf(stderr, "Configuring Game...\n");

    // Create Screenshots Folder
    char *screenshots_cmd = NULL;
    asprintf(&screenshots_cmd, "mkdir -p %s/.minecraft/screenshots", getenv("HOME"));
    system(screenshots_cmd);
    free(screenshots_cmd);

    char *ld_path = NULL;

    // Start Configuring LD_LIBRARY_PATH
    char *cwd = getcwd(NULL, 0);
    asprintf(&ld_path, "%s:/usr/arm-linux-gnueabihf/lib", cwd);
    free(cwd);

    // Start Configuring LD_PRELOAD
    char *ld_preload = NULL;
    asprintf(&ld_preload, "%s", get_env_safe("LD_PRELOAD"));

    // Load Mods From ./mods
    load(&ld_path, &ld_preload, "./mods/");

    // Loads Mods From ~/.minecraft/mods
    char *home_mods = NULL;
    asprintf(&home_mods, "%s/.minecraft/mods/", getenv("HOME"));
    load(&ld_path, &ld_preload, home_mods);
    free(home_mods);
    
    // Add Existing LD_LIBRARY_PATH
    asprintf(&ld_path, "%s:%s", ld_path, get_env_safe("LD_LIBRARY_PATH"));

    // Set LD_LIBRARY_PATH
    set_and_print_env("LD_LIBRARY_PATH", ld_path);
    free(ld_path);

    // Set LD_PRELOAD
    set_and_print_env("LD_PRELOAD", ld_preload);
    free(ld_preload);

    // Start Game
    fprintf(stderr, "Starting Game...\n");
    return execve("./minecraft-pi", argv, environ);
}
