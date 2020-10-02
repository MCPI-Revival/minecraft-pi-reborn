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

static void set_and_print_env(char *name, char *value) {
    int length = strlen(value);
    if (value[length - 1] == ':') {
        value[length - 1] = '\0';
    }

    fprintf(stderr, "Set %s = %s\n", name, value);
    setenv(name, value, 1);
}

static char *get_env_safe(const char *name) {
    char *ret = getenv(name);
    return ret != NULL ? ret : "";
}

static void load(char **ld_preload, char *folder) {
    int folder_name_length = strlen(folder);
    while (1) {
        DIR *dp = opendir(folder);
        if (dp != NULL) {
            struct dirent *entry = NULL;
            errno = 0;
            while (1) {
                entry = readdir(dp);
                if (entry != NULL) {
                    if (starts_with(entry->d_name, "lib") && ends_with(entry->d_name, ".so")) {
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

                        asprintf(ld_preload, "%s:%s", name, *ld_preload);
                    }
                } else if (errno != 0) {
                    fprintf(stderr, "Error Reading Directory: %s\n", strerror(errno));
                    exit(1);
                } else {
                    break;
                }
            }
            closedir(dp);
            return;
        } else if (errno == ENOENT) {
            char *cmd = NULL;
            asprintf(&cmd, "mkdir -p %s", folder);
            int ret = system(cmd);
            if (ret != 0) {
                exit(ret);
            }
        } else {
            fprintf(stderr, "Error Opening Directory: %s\n", strerror(errno));
            exit(1);
        }
    }
}

int main(__attribute__((unused)) int argc, char *argv[]) {
    fprintf(stderr, "Configuring Game...\n");

    char *ld_path = NULL;

    char *cwd = getcwd(NULL, 0);
    asprintf(&ld_path, "%s:/usr/arm-linux-gnueabihf/lib:%s", cwd, get_env_safe("LD_LIBRARY_PATH"));
    free(cwd);

    set_and_print_env("LD_LIBRARY_PATH", ld_path);
    free(ld_path);

    char *ld_preload = NULL;
    asprintf(&ld_preload, "%s", get_env_safe("LD_PRELOAD"));

    load(&ld_preload, "./mods/");

    char *home_mods = NULL;
    asprintf(&home_mods, "%s/.minecraft/mods/", getenv("HOME"));
    load(&ld_preload, home_mods);
    free(home_mods);

    set_and_print_env("LD_PRELOAD", ld_preload);
    free(ld_preload);

    fprintf(stderr, "Starting Game...\n");
    return execve("./minecraft-pi", argv, environ);
}
