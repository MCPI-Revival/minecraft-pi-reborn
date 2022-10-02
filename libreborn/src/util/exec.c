#include <pthread.h>

#include <libreborn/exec.h>

// Set Environmental Variable
static void setenv_safe(const char *name, const char *value) {
    if (value != NULL) {
        setenv(name, value, 1);
    } else {
        unsetenv(name);
    }
}
void set_and_print_env(const char *name, const char *value) {
    // Set The Value
    setenv_safe(name, value);

    // Print New Value
    DEBUG("Set %s = %s", name, value != NULL ? value : "(unset)");
}

// Safe execvpe()
#define handle_environmental_variable(var) \
    { \
        const char *full_var = is_arm_component ? "MCPI_ARM_" var : "MCPI_NATIVE_" var; \
        const char *var_value = getenv(full_var); \
        set_and_print_env(var, var_value); \
    }
void setup_exec_environment(int is_arm_component) {
    for_each_special_environmental_variable(handle_environmental_variable);
}
__attribute__((noreturn)) void safe_execvpe(const char *const argv[], const char *const envp[]) {
    // Log
    DEBUG("Running Command:");
    for (int i = 0; argv[i] != NULL; i++) {
        DEBUG("    %s", argv[i]);
    }
    // Run
    int ret = execvpe(argv[0], (char *const *) argv, (char *const *) envp);
    if (ret == -1) {
        ERR("Unable To Execute Program: %s: %s", argv[0], strerror(errno));
    } else {
        IMPOSSIBLE();
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

// Run Command And Get Output
char *run_command(const char *const command[], int *exit_status) {
    // Store Output
    int output_pipe[2];
    safe_pipe2(output_pipe, 0);
    // Run
    pid_t ret = fork();
    if (ret == -1) {
        ERR("Unable To Run Command: %s", strerror(errno));
    } else if (ret == 0) {
        // Child Process

        // Set Debug Tag
        reborn_debug_tag = CHILD_PROCESS_TAG;

        // Pipe stdout
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[0]);
        close(output_pipe[1]);

        // Setup stderr
        if (getenv("MCPI_DEBUG") == NULL) {
            const char *log_file_fd_env = getenv("MCPI_LOG_FILE_FD");
            if (log_file_fd_env == NULL) {
                IMPOSSIBLE();
            }
            dup2(atoi(log_file_fd_env), STDERR_FILENO);
        }

        // Setup Environment
        setup_exec_environment(0);

        // Run
        safe_execvpe(command, (const char *const *) environ);
    } else {
        // Parent Process
        track_child(ret);

        // Read stdout
        close(output_pipe[1]);
        char *output = NULL;
#define BUFFER_SIZE 1024
        char buf[BUFFER_SIZE];
        ssize_t bytes_read = 0;
        while ((bytes_read = read(output_pipe[0], (void *) buf, BUFFER_SIZE - 1 /* Account For NULL-Terminator */)) > 0) {
            buf[bytes_read] = '\0';
            string_append(&output, "%s", buf);
        }
        close(output_pipe[0]);

        // Get Return Code
        int status;
        waitpid(ret, &status, 0);
        untrack_child(ret);
        if (exit_status != NULL) {
            *exit_status = status;
        }

        // Return
        return output;
    }
}

// Get Exit Status String
void get_exit_status_string(int status, char **out) {
    if (out != NULL) {
        *out =NULL;
        if (WIFEXITED(status)) {
            safe_asprintf(out, ": Exit Code: %i", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            safe_asprintf(out, ": Signal: %i%s", WTERMSIG(status), WCOREDUMP(status) ? " (Core Dumped)" : "");
        } else {
            safe_asprintf(out, ": Terminated");
        }
    }
}

// Track Children
#define MAX_CHILDREN 128
static pid_t children[MAX_CHILDREN] = { 0 };
static pthread_mutex_t children_lock = PTHREAD_MUTEX_INITIALIZER;
void track_child(pid_t pid) {
    pthread_mutex_lock(&children_lock);
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (children[i] == 0) {
            children[i] = pid;
            break;
        }
    }
    pthread_mutex_unlock(&children_lock);
}
void untrack_child(pid_t pid) {
    pthread_mutex_lock(&children_lock);
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (children[i] == pid) {
            children[i] = 0;
        }
    }
    pthread_mutex_unlock(&children_lock);
}
void murder_children() {
    pthread_mutex_lock(&children_lock);
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (children[i] != 0) {
            kill(children[i], SIGTERM);
        }
    }
    pthread_mutex_unlock(&children_lock);
}
