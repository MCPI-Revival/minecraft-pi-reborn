#include <libreborn/libreborn.h>

#include "util.h"

// Simpler Version Of run_command()
void run_simple_command(const char *const command[], const char *error) {
    int status = 0;
    char *output = run_command(command, &status, NULL);
    if (output != NULL) {
        free(output);
    }
    if (!is_exit_status_success(status)) {
        ERR("%s", error);
    }
}

// Chop Off Last Component
void chop_last_component(char **str) {
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
char *get_binary_directory() {
    // Get Path To Current Executable
    char *exe = realpath("/proc/self/exe", NULL);
    ALLOC_CHECK(exe);

    // Chop Off Last Component
    chop_last_component(&exe);

    // Return
    return exe;
}
