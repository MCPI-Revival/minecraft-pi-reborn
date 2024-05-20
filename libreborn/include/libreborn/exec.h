#pragma once

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "log.h"
#include "string.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

// Set Environmental Variable
void set_and_print_env(const char *name, const char *value);

// Safe execvpe()
__attribute__((noreturn)) void safe_execvpe(const char *const argv[], const char *const envp[]);

// Debug Tag
#define CHILD_PROCESS_TAG "(Child Process) "

// Run Command And Get Output
char *run_command(const char *const command[], int *exit_status, size_t *output_size);
#define is_exit_status_success(status) (WIFEXITED(status) && WEXITSTATUS(status) == 0)

// Get Exit Status String
void get_exit_status_string(int status, char **out);

// Track Children
void track_child(pid_t pid);
void untrack_child(pid_t pid);
void murder_children();

#ifdef __cplusplus
}
#endif
