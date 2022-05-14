#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <poll.h>

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
        "--name", GUI_TITLE,
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
void setup_crash_report() {
    // Store Output
#ifndef MCPI_HEADLESS_MODE
    int output_pipe[2];
    safe_pipe2(output_pipe, 0);
    int error_pipe[2];
    safe_pipe2(error_pipe, 0);
#endif

    // Fork
    pid_t ret = fork();
    if (ret == -1) {
        ERR("Unable To Fork: %s", strerror(errno));
    } else if (ret == 0) {
        // Child Process

        // Pipe stdio
#ifndef MCPI_HEADLESS_MODE
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[0]);
        close(output_pipe[1]);
        dup2(error_pipe[1], STDERR_FILENO);
        close(error_pipe[0]);
        close(error_pipe[1]);
#endif

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

        // Capture stdio
#ifndef MCPI_HEADLESS_MODE
        // Close Unneeded File Descriptors
        close(output_pipe[1]);
        close(error_pipe[1]);

        // Create A Buffer
#define BUFFER_SIZE 1024
        char buf[BUFFER_SIZE];

        // Create Temporary File
        char log_filename[] = "/tmp/.minecraft-pi-log-XXXXXX";
        int log_file_fd = mkstemp(log_filename);
        if (log_file_fd == -1) {
            ERR("Unable To Create Log File: %s", strerror(errno));
        }

        // Setup Polling
        int number_fds = 2;
        struct pollfd poll_fds[number_fds];
        poll_fds[0].fd = output_pipe[0];
        poll_fds[1].fd = error_pipe[0];
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
                        // Data Available
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
                    } else {
                        // File Descriptor No Longer Accessible
                        if (close(poll_fds[i].fd) == -1) {
                            ERR("Unable To Close File Descriptor: %s", strerror(errno));
                        }
                        number_open_fds--;
                    }
                }
            }
        }
#endif

        // Get Return Code
        int status;
        waitpid(ret, &status, 0);
        untrack_child(ret);

        // Check If Is Crash
        int is_crash = WIFEXITED(status) ? WEXITSTATUS(status) != 0 : 1;

        // Log Exit Code To log If Crash
        if (is_crash) {
            // Create Exit Code Log Line
            char *exit_code_line = NULL;
            if (WIFEXITED(status)) {
                safe_asprintf(&exit_code_line, "[CRASH]: Terminated: Exit Code: %i\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                safe_asprintf(&exit_code_line, "[CRASH]: Terminated: Signal: %i%s\n", WTERMSIG(status), WCOREDUMP(status) ? " (Core Dumped)" : "");
            } else {
                safe_asprintf(&exit_code_line, "[CRASH]: Terminated\n");
            }

            // Print Exit Code Log Line
            fprintf(stderr, "%s", exit_code_line);

            // Write Exit Code Log Line
#ifndef MCPI_HEADLESS_MODE
            if (write(log_file_fd, (void *) exit_code_line, strlen(exit_code_line)) == -1) {
                ERR("Unable To Write Exit Code To Log: %s", strerror(errno));
            }
#endif

            // Free Exit Code Log Line
            free(exit_code_line);
        }

        // Show Crash Log
#ifndef MCPI_HEADLESS_MODE
        // Close Log File FD
        if (close(log_file_fd) == -1) {
            ERR("Unable To Close Log File Descriptor: %s", strerror(errno));
        }

        // Show Report
        if (is_crash) {
            show_report(log_filename);
        }

        // Delete Log File
        if (unlink(log_filename) == -1) {
            ERR("Unable To Delete Log File: %s", strerror(errno));
        }
#endif

        // Exit
        exit(WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE);
    }
}
