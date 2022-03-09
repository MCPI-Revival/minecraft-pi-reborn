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

// Chop Off Last Component
static inline void chop_last_component(char **str) {
    size_t length = strlen(*str);
    for (size_t i = 0; i < length; i++) {
        size_t j = length - i - 1;
        if ((*str)[j] == '/') {
            (*str)[j] = '\0';
            break;
        }
    }
}
// Get Binary Directory (Remember To Free)
static inline char *get_binary_directory() {
    // Get Path To Current Executable
    char *exe = realpath("/proc/self/exe", NULL);
    ALLOC_CHECK(exe);

    // Chop Off Last Component
    chop_last_component(&exe);

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

// Get MCPI Directory
static inline char *get_mcpi_directory() {
    return getenv("MCPI_DIRECTORY");
}
