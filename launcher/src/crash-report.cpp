#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <csignal>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <ctime>
#include <string>

#include <libreborn/libreborn.h>

#include "crash-report.h"

// Show Crash Report Dialog
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
            "--text", MCPI_APP_TITLE " has crashed!\n\nNeed help? Consider asking on the <a href=\"" MCPI_DISCORD_INVITE "\">Discord server</a>! <i>If you believe this is a problem with " MCPI_APP_TITLE " itself, please upload this crash report to the #bugs Discord channel.</i>",
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

// Exit Handler
static pid_t child_pid = -1;
static void exit_handler(__attribute__((unused)) int signal) {
    // Murder
    kill(child_pid, SIGTERM);
}

// Log File
static std::string log_filename;
static int log_fd;
static void setup_log_file() {
    // Get Log Directory
    const std::string home = std::string(getenv(_MCPI_HOME_ENV)) + get_home_subdirectory_for_game_data();
    ensure_directory(home.c_str());
    const std::string logs = home + "/logs";
    ensure_directory(logs.c_str());

    // Get Timestamp
    time_t raw_time;
    time(&raw_time);
    const tm *time_info = localtime(&raw_time);
    char time[512];
    strftime(time, 512, "%Y-%m-%d", time_info);

    // Get Log Filename
    std::string file;
    int num = 1;
    do {
        file = std::string(time) + '-' + std::to_string(num) + ".log";
        log_filename = logs + '/' + file;
        num++;
    } while (access(log_filename.c_str(), F_OK) != -1);

    // Create latest.log Symlink
    const std::string latest_log = logs + "/latest.log";
    unlink(latest_log.c_str());
    if (symlink(file.c_str(), latest_log.c_str()) != 0) {
        WARN("Unable To Create Latest Log Symlink: %s", strerror(errno));
    }

    // Create File
    log_fd = open(log_filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (log_fd == -1) {
        ERR("Unable To Create Log File: %s", strerror(errno));
    }
    reborn_set_log(log_fd);
}

// Setup
#define PIPE_READ 0
#define PIPE_WRITE 1
#define BUFFER_SIZE 1024
static void safe_write(int fd, const void *buf, size_t size) {
    const ssize_t bytes_written = write(fd, buf, size);
    if (bytes_written < 0) {
        ERR("Unable To Write Data: %s", strerror(errno));
    }
}
void setup_crash_report() {
    // Setup Logging
    setup_log_file();

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

        // Kill Child If Parent Exits First
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        // Continue Execution
    } else {
        // Install Signal Handlers
        child_pid = ret;
        struct sigaction act_sigint = {};
        act_sigint.sa_flags = SA_RESTART;
        act_sigint.sa_handler = &exit_handler;
        sigaction(SIGINT, &act_sigint, nullptr);
        struct sigaction act_sigterm = {};
        act_sigterm.sa_flags = SA_RESTART;
        act_sigterm.sa_handler = &exit_handler;
        sigaction(SIGTERM, &act_sigterm, nullptr);

        // Close Unneeded File Descriptors
        close(output_pipe[PIPE_WRITE]);
        close(error_pipe[PIPE_WRITE]);
        close(input_pipe[PIPE_READ]);

        // Set Debug Tag
        reborn_debug_tag = "(Crash Reporter) ";

        // Setup Polling
        const int number_fds = 3;
        pollfd poll_fds[number_fds];
        poll_fds[0].fd = output_pipe[PIPE_READ];
        poll_fds[1].fd = error_pipe[PIPE_READ];
        poll_fds[2].fd = STDIN_FILENO;
        for (pollfd &poll_fd : poll_fds) {
            poll_fd.events = POLLIN;
        }

        // Poll Data
        int status;
        while (waitpid(ret, &status, WNOHANG) != ret) {
            const int poll_ret = poll(poll_fds, number_fds, -1);
            if (poll_ret == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    ERR("Unable To Poll Data: %s", strerror(errno));
                }
            }

            // Handle Data
            for (pollfd &poll_fd : poll_fds) {
                if (poll_fd.revents != 0) {
                    if (poll_fd.revents & POLLIN) {
                        char buf[BUFFER_SIZE];
                        if (poll_fd.fd == STDIN_FILENO) {
                            // Data Available From stdin
                            int bytes_available;
                            if (ioctl(fileno(stdin), FIONREAD, &bytes_available) == -1) {
                                bytes_available = 0;
                            }
                            // Read
                            const ssize_t bytes_read = read(poll_fd.fd, buf, BUFFER_SIZE);
                            if (bytes_read == -1) {
                                ERR("Unable To Read Input: %s", strerror(errno));
                            }
                            // Write To Child
                            safe_write(input_pipe[PIPE_WRITE], buf, bytes_read);
                        } else {
                            // Data Available From Child's stdout/stderr
                            const ssize_t bytes_read = read(poll_fd.fd, buf, BUFFER_SIZE);
                            if (bytes_read == -1) {
                                ERR("Unable To Read Log Data: %s", strerror(errno));
                            }
                            // Print To Terminal
                            safe_write(poll_fd.fd == output_pipe[PIPE_READ] ? STDOUT_FILENO : STDERR_FILENO, buf, bytes_read);
                            // Write To log
                            safe_write(reborn_get_log_fd(), buf, bytes_read);
                        }
                    } else {
                        // File Descriptor No Longer Accessible
                        poll_fd.events = 0;
                    }
                }
            }
        }

        // Close Pipes
        close(output_pipe[PIPE_READ]);
        close(error_pipe[PIPE_READ]);
        close(input_pipe[PIPE_WRITE]);

        // Check If Is Crash
        const bool is_crash = !is_exit_status_success(status);

        // Log Exit Code To log If Crash
        if (is_crash) {
            // Create Exit Code Log Line
            char *exit_status = nullptr;
            get_exit_status_string(status, &exit_status);
            const std::string exit_code_line = "[CRASH]: Terminated" + std::string(exit_status) + '\n';
            free(exit_status);

            // Print Exit Code Log Line
            safe_write(STDERR_FILENO, exit_code_line.c_str(), strlen(exit_code_line.c_str()));
            // Write Exit Code Log Line
            safe_write(reborn_get_log_fd(), exit_code_line.c_str(), strlen(exit_code_line.c_str()));
        }

        // Close Log File
        close(log_fd);
        unsetenv(_MCPI_LOG_FD_ENV);

        // Show Crash Log
        if (is_crash && !reborn_is_headless()) {
            show_report(log_filename.c_str());
        }

        // Exit
        exit(WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE);
    }
}
