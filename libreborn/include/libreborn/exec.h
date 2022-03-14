#pragma once

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "log.h"
#include "string.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

// Safe execvpe()
__attribute__((noreturn)) void safe_execvpe(const char *const argv[], const char *const envp[]);

// Chop Off Last Component
void chop_last_component(char **str);
// Get Binary Directory (Remember To Free)
char *get_binary_directory();

// Safe execvpe() Relative To Binary
__attribute__((noreturn)) void safe_execvpe_relative_to_binary(const char *const argv[], const char *const envp[]);

// Get MCPI Directory
char *get_mcpi_directory();

// Run Command And Get Output
char *run_command(const char *const command[], int *return_code);

#ifdef __cplusplus
}
#endif
