#pragma once

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "log.h"
#include "string.h"
#include "util.h"

// Safe execvpe()
__attribute__((noreturn)) static inline void safe_execvpe(const char *pathname, char *argv[], char *const envp[]) {
    argv[0] = (char *) pathname;
    int ret = execvpe(pathname, argv, envp);
    if (ret == -1) {
        ERR("Unable To Execute Program: %s: %s", pathname, strerror(errno));
    } else {
        ERR("%s", "Unknown execvpe() Error");
    }
}
// Get Binary Directory (Remember To Free)
static inline char *get_binary_directory() {
    // Get Path To Current Executable
    char *exe = realpath("/proc/self/exe", NULL);
    ALLOC_CHECK(exe);

    // Chop Off Last Component
    int length = strlen(exe);
    for (int i = length - 1; i >= 0; i--) {
        if (exe[i] == '/') {
            exe[i] = '\0';
            break;
        }
    }

    // Return
    return exe;
}
// Safe execvpe() Relative To Binary
__attribute__((noreturn)) static inline void safe_execvpe_relative_to_binary(const char *pathname, char *argv[], char *const envp[]) {
    // Get Binary Directory
    char *binary_directory = get_binary_directory();
    // Create Full Path
    char *full_path = NULL;
    safe_asprintf(&full_path, "%s/%s", binary_directory, pathname);
    // Free Binary Directory
    free(binary_directory);
    // Run
    safe_execvpe(full_path, argv, envp);
}
