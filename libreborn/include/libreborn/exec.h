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
#define for_each_special_environmental_variable(handle) \
    handle("LD_LIBRARY_PATH"); \
    handle("GCONV_PATH"); \
    handle("LD_PRELOAD");
void setup_exec_environment(int is_arm_component);
__attribute__((noreturn)) void safe_execvpe(const char *const argv[], const char *const envp[]);

// Chop Off Last Component
void chop_last_component(char **str);
// Get Binary Directory (Remember To Free)
char *get_binary_directory();

// Debug Tag
#define CHILD_PROCESS_TAG "(Child Process) "

// Run Command And Get Output
char *run_command(const char *const command[], int *exit_status);
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
