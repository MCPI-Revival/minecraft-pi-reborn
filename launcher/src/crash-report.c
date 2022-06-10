#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <poll.h>
#include <sys/ioctl.h>

#include <libreborn/libreborn.h>

#include "crash-report.h"

// Show Crash Report Dialog
#ifndef MCPI_HEADLESS_MODE
#define DIALOG_TITLE "Crash Report"
#define CRASH_REPORT_DIALOG_WIDTH "640"
#define CRASH_REPORT_DIALOG_HEIGHT "480"
static void show_report(const char *log_filename) {
    const char *command[] = {
        "zenity",
        "--title", DIALOG_TITLE,
        "--name", MCPI_APP_TITLE,
        "--width", CRASH_REPORT_DIALOG_WIDTH,
        "--height", CRASH_REPORT_DIALOG_HEIGHT,
        "--text-info",
        "--text", "Minecraft: Pi Edition: Reborn has crashed!\n\nNeed help? Consider asking on the <a href=\"https://discord.com/invite/aDqejQGMMy\">Discord server</a>!",
        "--filename", log_filename,
        "--no-wrap",
        "--font", "Monospace",
        NULL
    };
    free(run_command(command, NULL));
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
        struct sigaction act_sigint;
        memset((void *) &act_sigint, 0, sizeof (struct sigaction));
        act_sigint.sa_flags = SA_RESTART;
        act_sigint.sa_handler = &exit_handler;
        sigaction(SIGINT, &act_sigint, NULL);
        struct sigaction act_sigterm;
        memset((void *) &act_sigterm, 0, sizeof (struct sigaction));
        act_sigterm.sa_flags = SA_RESTART;
        act_sigterm.sa_handler = &exit_handler;
        sigaction(SIGTERM, &act_sigterm, NULL);

        // Close Unneeded File Descriptors
        close(output_pipe[PIPE_WRITE]);
        close(error_pipe[PIPE_WRITE]);
        close(input_pipe[PIPE_READ]);

        // Setup Logging
#define BUFFER_SIZE 1024
        char buf[BUFFER_SIZE];

        // Create Temporary File
        char log_filename[] = "/tmp/.minecraft-pi-log-XXXXXX";
        int log_file_fd = mkstemp(log_filename);
        if (log_file_fd == -1) {
            ERR("Unable To Create Log File: %s", strerror(errno));
        }

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
        int number_open_fds = number_fds;
        while (number_open_fds > 0) {
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
                            ssize_t bytes_read = read(poll_fds[i].fd, (void *) buf, BUFFER_SIZE);
                            if (bytes_read == -1) {
                                ERR("Unable To Read Log Data: %s", strerror(errno));
                            }
                            // Write To Child
                            if (write(input_pipe[PIPE_WRITE], (void *) buf, bytes_read) == -1) {
                                ERR("Unable To Write Input To Child: %s", strerror(errno));
                            }
                        } else {
                            // Data Available From Child's stdout/stderr
                            ssize_t bytes_read = read(poll_fds[i].fd, (void *) buf, BUFFER_SIZE - 1 /* Account For NULL-Terminator */);
                            if (bytes_read == -1) {
                                ERR("Unable To Read Log Data: %s", strerror(errno));
                            }

                            // Print To Terminal
                            buf[bytes_read] = '\0';
                            fprintf(i == 0 ? stdout : stderr, "%s", buf);

                            // Write To log
                            if (write(log_file_fd, (void *) buf, bytes_read) == -1) {
                                ERR("Unable To Write Log Data: %s", strerror(errno));
                            }
                        }
                    } else {
                        // File Descriptor No Longer Accessible
                        if (poll_fds[i].events != 0 && close(poll_fds[i].fd) == -1) {
                            ERR("Unable To Close File Descriptor: %s", strerror(errno));
                        }
                        poll_fds[i].events = 0;
                        number_open_fds--;
                    }
                }
            }
        }

        // Close Input Pipe
        close(input_pipe[PIPE_WRITE]);

        // Get Return Code
        int status;
        waitpid(ret, &status, 0);
        untrack_child(ret);

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
            if (write(log_file_fd, (void *) exit_code_line, strlen(exit_code_line)) == -1) {
                ERR("Unable To Write Exit Code To Log: %s", strerror(errno));
            }

            // Free Exit Code Log Line
            free(exit_code_line);
        }

        // Close Log File FD
        if (close(log_file_fd) == -1) {
            ERR("Unable To Close Log File Descriptor: %s", strerror(errno));
        }

        // Show Crash Log
#ifndef MCPI_HEADLESS_MODE
        if (is_crash) {
            show_report(log_filename);
        }
#endif

        // Delete Log File
        if (unlink(log_filename) == -1) {
            ERR("Unable To Delete Log File: %s", strerror(errno));
        }

        // Exit
        exit(WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE);
    }
}
