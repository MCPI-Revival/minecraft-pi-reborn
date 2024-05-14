#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <libreborn/libreborn.h>

#include "crash-report.h"

// Show Crash Report Dialog
#ifndef MCPI_HEADLESS_MODE
#define DIALOG_TITLE "Crash Report"
#define CRASH_REPORT_DIALOG_WIDTH "640"
#define CRASH_REPORT_DIALOG_HEIGHT "480"
static void show_report(const char *log_filename) {
    // Fork
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        setsid();
        ALLOC_CHECK(freopen("/dev/null", "w", stdout));
        ALLOC_CHECK(freopen("/dev/null", "w", stderr));
        ALLOC_CHECK(freopen("/dev/null", "r", stdin));
        const char *command[] = {
            "zenity",
            "--title", DIALOG_TITLE,
            "--name", MCPI_APP_ID,
            "--width", CRASH_REPORT_DIALOG_WIDTH,
            "--height", CRASH_REPORT_DIALOG_HEIGHT,
            "--text-info",
            "--text", MCPI_APP_BASE_TITLE " has crashed!\n\nNeed help? Consider asking on the <a href=\"" MCPI_DISCORD_INVITE "\">Discord server</a>! <i>If you believe this is a problem with " MCPI_APP_BASE_TITLE " itself, please upload this crash report to the #bugs Discord channel.</i>",
            "--filename", log_filename,
            "--no-wrap",
            "--font", "Monospace",
            "--save-filename", MCPI_VARIANT_NAME "-crash-report.log",
            "--ok-label", "Exit",
            NULL
        };
        safe_execvpe(command, (const char *const *) environ);
    }
}
#endif

// Exit Handler
static void exit_handler(__attribute__((unused)) int signal) {
    // Murder
    murder_children();
}

// Setup
#define PIPE_READ 0
#define PIPE_WRITE 1
#define MCPI_LOGS_DIR "/tmp/.minecraft-pi-logs"
static char log_filename[] = MCPI_LOGS_DIR "/XXXXXX";
void setup_log_file() {
    // Ensure Temporary Directory
    {
        // Check If It Exists
        struct stat tmp_stat;
        int exists = stat(MCPI_LOGS_DIR, &tmp_stat) != 0 ? 0 : S_ISDIR(tmp_stat.st_mode);
        if (!exists) {
            // Doesn't Exist
            if (mkdir(MCPI_LOGS_DIR, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
                ERR("Unable To Create Temporary Folder: %s", strerror(errno));
            }
        }
    }

    // Create Temporary File
    int log_file_fd = mkstemp(log_filename);
    if (log_file_fd == -1) {
        ERR("Unable To Create Log File: %s", strerror(errno));
    }
    close(log_file_fd);
    reborn_set_log(log_filename);
}
void setup_crash_report() {
    // Store Output
    int output_pipe[2];
    safe_pipe2(output_pipe, 0);
    int error_pipe[2];
    safe_pipe2(error_pipe, 0);
    int input_pipe[2];
    safe_pipe2(input_pipe, 0);

    // Fork
    pid_t ret = fork();
    if (ret == -1) {
        ERR("Unable To Fork: %s", strerror(errno));
    } else if (ret == 0) {
        // Child Process

        // Pipe stdio
        dup2(output_pipe[PIPE_WRITE], STDOUT_FILENO);
        close(output_pipe[PIPE_READ]);
        close(output_pipe[PIPE_WRITE]);
        dup2(error_pipe[PIPE_WRITE], STDERR_FILENO);
        close(error_pipe[PIPE_READ]);
        close(error_pipe[PIPE_WRITE]);
        dup2(input_pipe[PIPE_READ], STDIN_FILENO);
        close(input_pipe[PIPE_READ]);
        close(input_pipe[PIPE_WRITE]);

        // Create New Process Group
        setpgid(0, 0);

        // Continue Execution
    } else {
        // Parent Process
        track_child(ret);

        // Install Signal Handlers
        struct sigaction act_sigint = {0};
        act_sigint.sa_flags = SA_RESTART;
        act_sigint.sa_handler = &exit_handler;
        sigaction(SIGINT, &act_sigint, NULL);
        struct sigaction act_sigterm = {0};
        act_sigterm.sa_flags = SA_RESTART;
        act_sigterm.sa_handler = &exit_handler;
        sigaction(SIGTERM, &act_sigterm, NULL);
        atexit(murder_children);

        // Close Unneeded File Descriptors
        close(output_pipe[PIPE_WRITE]);
        close(error_pipe[PIPE_WRITE]);
        close(input_pipe[PIPE_READ]);

        // Set Debug Tag
        reborn_debug_tag = "(Crash Reporter) ";

        // Setup Logging
#define BUFFER_SIZE 1024
        char buf[BUFFER_SIZE];

        // Setup Polling
        int number_fds = 3;
        struct pollfd poll_fds[number_fds];
        poll_fds[0].fd = output_pipe[PIPE_READ];
        poll_fds[1].fd = error_pipe[PIPE_READ];
        poll_fds[2].fd = STDIN_FILENO;
        for (int i = 0; i < number_fds; i++) {
            poll_fds[i].events = POLLIN;
        }

        // Poll Data
        int status;
        while (waitpid(ret, &status, WNOHANG) != ret) {
            int poll_ret = poll(poll_fds, number_fds, -1);
            if (poll_ret == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    ERR("Unable To Poll Data: %s", strerror(errno));
                }
            }

            // Handle Data
            for (int i = 0; i < number_fds; i++) {
                if (poll_fds[i].revents != 0) {
                    if (poll_fds[i].revents & POLLIN) {
                        if (poll_fds[i].fd == STDIN_FILENO) {
                            // Data Available From stdin
                            int bytes_available;
                            if (ioctl(fileno(stdin), FIONREAD, &bytes_available) == -1) {
                                bytes_available = 0;
                            }
                            // Read
                            ssize_t bytes_read = read(poll_fds[i].fd, buf, BUFFER_SIZE);
                            if (bytes_read == -1) {
                                ERR("Unable To Read Input: %s", strerror(errno));
                            }
                            // Write To Child
                            if (write(input_pipe[PIPE_WRITE], buf, bytes_read) == -1) {
                                ERR("Unable To Write Input To Child: %s", strerror(errno));
                            }
                        } else {
                            // Data Available From Child's stdout/stderr
                            ssize_t bytes_read = read(poll_fds[i].fd, buf, BUFFER_SIZE - 1 /* Account For NULL-Terminator */);
                            if (bytes_read == -1) {
                                ERR("Unable To Read Log Data: %s", strerror(errno));
                            }

                            // Print To Terminal
                            buf[bytes_read] = '\0';
                            fprintf(poll_fds[i].fd == output_pipe[PIPE_READ] ? stdout : stderr, "%s", buf);

                            // Write To log
                            reborn_lock_log();
                            if (write(reborn_get_log_fd(), buf, bytes_read) == -1) {
                                ERR("Unable To Write Log Data: %s", strerror(errno));
                            }
                            reborn_unlock_log();
                        }
                    } else {
                        // File Descriptor No Longer Accessible
                        poll_fds[i].events = 0;
                    }
                }
            }
        }

        // Untrack Process
        untrack_child(ret);

        // Close Pipes
        close(output_pipe[PIPE_READ]);
        close(error_pipe[PIPE_READ]);
        close(input_pipe[PIPE_WRITE]);

        // Check If Is Crash
        int is_crash = !is_exit_status_success(status);

        // Log Exit Code To log If Crash
        if (is_crash) {
            // Create Exit Code Log Line
            char *exit_status = NULL;
            get_exit_status_string(status, &exit_status);
            char *exit_code_line = NULL;
            safe_asprintf(&exit_code_line, "[CRASH]: Terminated%s\n", exit_status);
            free(exit_status);

            // Print Exit Code Log Line
            fprintf(stderr, "%s", exit_code_line);

            // Write Exit Code Log Line
            reborn_lock_log();
            if (write(reborn_get_log_fd(), exit_code_line, strlen(exit_code_line)) == -1) {
                ERR("Unable To Write Exit Code To Log: %s", strerror(errno));
            }
            reborn_unlock_log();

            // Free Exit Code Log Line
            free(exit_code_line);
        }

        // Close Log File
        reborn_close_log();

        // Show Crash Log
#ifndef MCPI_HEADLESS_MODE
        if (is_crash) {
            show_report(log_filename);
        }
#endif

        // Exit
        exit(WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE);
    }
}
