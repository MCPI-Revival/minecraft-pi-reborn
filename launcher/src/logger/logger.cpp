#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <sys/stat.h>
#include <string>
#include <fcntl.h>

#include <libreborn/util/exec.h>
#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/string.h>
#include <libreborn/util/io.h>
#include <libreborn/config.h>

#include "logger.h"

// Exit Handler
static pid_t child_pid = -1;
static void exit_handler(__attribute__((unused)) int signal) {
    // Murder
    kill(child_pid, SIGTERM);
}

// Log File
static std::string log_filename;
static int log_fd;
std::string get_logs_folder() {
    const std::string home = home_get();
    ensure_directory(home.c_str());
    const std::string logs = home + "/logs";
    ensure_directory(logs.c_str());
    return logs;
}
static void setup_log_file() {
    // Get Log Directory
    const std::string logs = get_logs_folder();

    // Get Timestamp
    const std::string time = format_time("%Y-%m-%d");

    // Get Log Filename
    std::string file;
    int num = 1;
    do {
        file = time + '-' + std::to_string(num) + ".log";
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
void setup_logger() {
    // Setup Logging
    setup_log_file();

    // Fork
    std::optional<Process> child = fork_with_stdio();
    if (!child) {
        // Child Process

        // Create New Process Group
        setpgid(0, 0);

        // Continue Execution
    } else {
        // Set Debug Tag
        reborn_debug_tag = "(Logger) ";
        DEBUG("Writing To: %s", log_filename.c_str());

        // Install Signal Handlers
        child_pid = child->pid;
        struct sigaction act_sigint = {};
        act_sigint.sa_flags = SA_RESTART;
        act_sigint.sa_handler = &exit_handler;
        sigaction(SIGINT, &act_sigint, nullptr);
        struct sigaction act_sigterm = {};
        act_sigterm.sa_flags = SA_RESTART;
        act_sigterm.sa_handler = &exit_handler;
        sigaction(SIGTERM, &act_sigterm, nullptr);

        // Poll
        poll_fds({child->fds[0], child->fds[1], STDIN_FILENO}, [&child](const int i, const size_t size, unsigned char *buf) {
            if (i == 0 || i == 1) {
                // stdout/stderr

                // Print To Terminal
                safe_write(i == 0 ? STDOUT_FILENO : STDERR_FILENO, buf, size);
                // Write To log
                safe_write(reborn_get_log_fd(), buf, size);
            } else {
                // stdin

                // Write To Child
                safe_write(child->fds[2], buf, size);
            }
        });

        // Get Exit Status
        const int status = child->close();
        const bool is_crash = !is_exit_status_success(status);

        // Log Exit Code To log If Crash
        if (is_crash) {
            // Create Exit Code Log Line
            const std::string exit_status = get_exit_status_string(status);
            const std::string exit_code_line = "[CRASH]: Terminated" + exit_status + '\n';

            // Print Exit Code Log Line
            safe_write(STDERR_FILENO, exit_code_line.c_str(), exit_code_line.size());
            // Write Exit Code Log Line
            safe_write(reborn_get_log_fd(), exit_code_line.c_str(), exit_code_line.size());
        }

        // Close Log File
        close(log_fd);
        reborn_set_log(-1);

        // Show Crash Log
        if (is_crash && !reborn_is_headless()) {
            show_report(log_filename.c_str());
        }

        // Exit
        exit(WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE);
    }
}
