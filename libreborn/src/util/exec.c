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

// Run Command And Get Output
char *run_command(const char *const command[], int *exit_status, size_t *output_size) {
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
        reborn_lock_debug(); // Lock Released On Process Exit
        dup2(reborn_get_debug_fd(), STDERR_FILENO);

        // Run
        safe_execvpe(command, (const char *const *) environ);
    } else {
        // Parent Process
        track_child(ret);

        // Read stdout
        close(output_pipe[1]);
#define BUFFER_SIZE 1024
        size_t size = BUFFER_SIZE;
        char *output = malloc(size);
        char buf[BUFFER_SIZE];
        size_t position = 0;
        ssize_t bytes_read = 0;
        while ((bytes_read = read(output_pipe[0], buf, BUFFER_SIZE)) > 0) {
            // Grow Output If Needed
            size_t needed_size = position + bytes_read;
            if (needed_size > size) {
                // More Memeory Needed
                size_t new_size = size;
                while (new_size < needed_size) {
                    new_size += BUFFER_SIZE;
                }
                char *new_output = realloc(output, new_size);
                if (new_output == NULL) {
                    ERR("Unable To Grow Output Buffer");
                } else {
                    output = new_output;
                    size = new_size;
                }
            }

            // Copy Data To Output
            memcpy(output + position, buf, bytes_read);
            position += bytes_read;
        }
        close(output_pipe[0]);

        // Add NULL-Terminator To Output
        size_t needed_size = position + 1;
        if (needed_size > size) {
            // More Memeory Needed
            size_t new_size = size + 1;
            char *new_output = realloc(output, new_size);
            if (new_output == NULL) {
                ERR("Unable To Grow Output Buffer (For NULL-Terminator)");
            } else {
                output = new_output;
            }
        }
        output[position++] = '\0';

        // Return Output Size
        if (output_size != NULL) {
            *output_size = position;
        }

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
