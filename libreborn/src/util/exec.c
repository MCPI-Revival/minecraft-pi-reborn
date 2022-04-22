#include <libreborn/exec.h>

// Safe execvpe()
__attribute__((noreturn)) void safe_execvpe(const char *const argv[], const char *const envp[]) {
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

// Safe execvpe() Relative To Binary
__attribute__((noreturn)) void safe_execvpe_relative_to_binary(const char *const argv[], const char *const envp[]) {
    // Get Binary Directory
    char *binary_directory = get_binary_directory();
    // Create Full Path
    char *full_path = NULL;
    safe_asprintf(&full_path, "%s/%s", binary_directory, argv[0]);
    // Free Binary Directory
    free(binary_directory);

    // Build New argv
    int argc;
    for (argc = 0; argv[argc] != NULL; argc++);
    const char *new_argv[argc + 1];
    for (int i = 1; i < argc; i++) {
        new_argv[i] = argv[i];
    }
    new_argv[0] = full_path;
    new_argv[argc] = NULL;
    // Run
    safe_execvpe(new_argv, envp);
}

// Run Command And Get Output
char *run_command(const char *const command[], int *return_code) {
    // Store Output
    int output_pipe[2];
    safe_pipe2(output_pipe, 0);
    // Run
    pid_t ret = fork();
    if (ret == -1) {
        ERR("Unable To Run Command: %s", strerror(errno));
    } else if (ret == 0) {
        // Child Process

        // Pipe stdout
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[0]);
        close(output_pipe[1]);

        // Close stderr (But Not In Debug Mode)
        const char *is_debug = getenv("MCPI_DEBUG");
        if (is_debug == NULL || strlen(is_debug) < 1) {
            int null_fd = open("/dev/null", O_WRONLY);
            dup2(null_fd, STDERR_FILENO);
            close(null_fd);
        }

        // Run
        safe_execvpe(command, (const char *const *) environ);
    } else {
        // Parent Process

        // Read stdout
        close(output_pipe[1]);
        char *output = NULL;
#define BUFFER_SIZE 1024
        char buf[BUFFER_SIZE];
        size_t bytes_read = 0;
        while ((bytes_read = read(output_pipe[0], (void *) buf, BUFFER_SIZE - 1 /* Account For NULL-Terminator */)) > 0) {
            buf[bytes_read] = '\0';
            string_append(&output, "%s", buf);
        }
        close(output_pipe[0]);

        // Get Return Code
        int status;
        waitpid(ret, &status, 0);
        *return_code = WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE;

        // Return
        return output;
    }
}
