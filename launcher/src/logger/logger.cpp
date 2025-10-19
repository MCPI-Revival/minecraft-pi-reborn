#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <csignal>
#include <string>

#include <libreborn/util/exec.h>
#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/config.h>

#include "writer.h"
#include "../util/util.h"

// Exit Handler
static pid_t child_pid = -1;
static void exit_handler(MCPI_UNUSED int signal) {
    // Murder
    kill(child_pid, SIGTERM);
}

// Log File
static std::string get_logs_folder() {
    const std::string home = home_get();
    ensure_directory(home.c_str());
    const std::string logs = home + "/logs";
    ensure_directory(logs.c_str());
    return logs;
}
static LogWriter *log;
static Pipe *log_pipe;
static void setup_log_file() {
    // Get Log Directory
    const std::string logs = get_logs_folder();

    // Create File
    log = new LogWriter(logs);

    // Create Pipe
    log_pipe = new Pipe();
    reborn_set_log(log_pipe->write);

    // Create Symlink To Latest Log
    const std::string latest_log = logs + "/latest.log";
    unlink(latest_log.c_str());
    if (symlink(log->name.c_str(), latest_log.c_str()) != 0) {
        WARN("Unable To Create Latest Log Symlink: %s", strerror(errno));
    }
}

// Show Crash Report
static void show_report(const std::string &filename) {
    const std::string exe = get_binary_directory() + "/crash-report";
    const std::string dir = get_logs_folder();
    const char *argv[] = {exe.c_str(), filename.c_str(), dir.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(argv, nullptr);
    delete output;
}

// Setup
static void setup_logger_child() {
    // This process will run the actual program.

    // Close Unneeded FD
    close(log_pipe->read);

    // Create New Process Group
    setpgid(0, 0);
}
static int setup_logger_parent(Process &child) {
    // This process will:
    // * Forward the child's output to the terminal and the log file.
    // * Forward the terminal's input to the child.
    // * Display the crash report dialog when needed.

    // Set Debug Tag
    reborn_debug_tag = "(Logger) ";
    DEBUG("Writing To: %s", log->name.c_str());

    // Install Signal Handlers
    child_pid = child.pid;
    for (const int signal : {SIGINT, SIGTERM}) {
        struct sigaction action = {};
        action.sa_flags = SA_RESTART;
        action.sa_handler = &exit_handler;
        sigaction(signal, &action, nullptr);
    }

    // Close Unneeded FD
    close(log_pipe->write);

    // Poll
    const std::vector fds = {
        child.fds[0],
        child.fds[1],
        log_pipe->read,
        STDIN_FILENO
    };
    poll_fds(fds, [&child](const int i, const size_t size, unsigned char *buf) {
        switch (i) {
            case 0:
            case 1: {
                // Source: Child's stdout/stderr
                // Action: Print to the terminal
                const int target = i == 0 ? STDOUT_FILENO : STDERR_FILENO;
                safe_write(target, buf, size);
                [[fallthrough]];
            }
            case 2: {
                // Source: Child's debug log
                // Action: Write to the log file
                log->write(buf, std::streamsize(size), false);
                break;
            }
            case 3: {
                // Source: Parent's stdin
                // Action: Write to the child's stdin
                safe_write(child.fds[2], buf, size);
                break;
            }
            default: {}
        }
    });

    // Get Exit Status
    const int status = child.close();
    const bool is_crash = !is_exit_status_success(status);

    // Log Exit Code To The Log If Crash
    if (is_crash) {
        // Create Exit Code Log Line
        const std::string exit_status = get_exit_status_string(status);
        const char *exit_code_line = reborn_get_crash_message(exit_status.c_str());

        // Print Exit Code Log Line
        const unsigned char *data = (const unsigned char *) exit_code_line;
        const std::streamsize size = std::streamsize(strlen(exit_code_line));
        safe_write(STDERR_FILENO, data, size);
        // Write Exit Code Log Line
        log->write(data, size, true);
    }

    // Close Log File
    delete log_pipe;
    const std::string log_filename = log->name;
    delete log;
    reborn_set_log(-1);

    // Show Crash Log
    if (is_crash && !reborn_is_headless()) {
        show_report(log_filename);
    }

    // Exit
    return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE;
}

// Main
int main(MCPI_UNUSED const int argc, char *argv[]) {
    // Setup Logging
    setup_log_file();

    // Fork
    std::optional<Process> child = fork_with_stdio();
    if (!child) {
        // Child Process
        setup_logger_child();
        argv++;
        safe_execvpe(argv);
    } else {
        // Parent Process
        return setup_logger_parent(*child);
    }
}
