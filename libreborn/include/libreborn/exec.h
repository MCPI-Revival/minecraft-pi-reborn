#pragma once

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
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
#define EXE_PATH "/proc/self/exe"
static inline char *get_binary_directory() {
    // Get Path To Current Executable
    // Get Symlink Path Size
    struct stat sb;
    if (lstat(EXE_PATH, &sb) == -1) {
        ERR("Unable To Get " EXE_PATH " Symlink Size: %s", strerror(errno));
    }
    ssize_t path_size = sb.st_size + 1;
    if (sb.st_size == 0) {
        path_size = PATH_MAX;
    }
    char *exe = (char *) malloc(path_size);
    ALLOC_CHECK(exe);
    // Read Link
    ssize_t r = readlink(EXE_PATH, exe, path_size);
    if (r < 0) {
        ERR("Unable To Read " EXE_PATH " Symlink: %s", strerror(errno));
    }
    if (r > path_size) {
        ERR("%s", "Size Of Symlink " EXE_PATH " Changed");
    }
    exe[path_size] = '\0';

    // Chop Off Last Component
    for (int i = path_size - 1; i >= 0; i--) {
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
