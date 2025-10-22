#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <string>

#ifndef _WIN32
#include <csignal>
#endif

#include <libreborn/util/exec.h>
#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/config.h>

#include "writer.h"
#include "../util/util.h"

// Exit Handler
#ifndef _WIN32
static pid_t child_pid = -1;
static void exit_handler(MCPI_UNUSED int signal) {
    // Murder
    kill(child_pid, SIGTERM);
}
#endif

// Log File
static std::string get_logs_folder() {
    const std::string home = home_get();
    ensure_directory(home.c_str());
    const std::string logs = home + path_separator + "logs";
    ensure_directory(logs.c_str());
    return logs;
}
static LogWriter *log;
#ifndef _WIN32
static Pipe *log_pipe;
#endif
static void setup_log_file() {
    // Get Log Directory
    const std::string logs = get_logs_folder();

    // Create File
    log = new LogWriter(logs);

#ifndef _WIN32
    // Create Pipe
    log_pipe = new Pipe();
    reborn_set_log(log_pipe->write);

    // Create Symlink To Latest Log
    const std::string latest_log = logs + "/latest.log";
    unlink(latest_log.c_str());
    if (symlink(log->name.c_str(), latest_log.c_str()) != 0) {
        WARN("Unable To Create Latest Log Symlink: %s", strerror(errno));
    }
#endif
}

// Show Crash Report
static void show_report(const std::string &filename) {
    const std::string exe = get_binary_directory() + path_separator + "crash-report";
    const std::string dir = get_logs_folder();
    const char *argv[] = {exe.c_str(), filename.c_str(), dir.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(argv, nullptr);
    delete output;
}

// Setup
#ifndef _WIN32
static void setup_logger_child() {
    // This process will run the actual program.

    // Close Unneeded FD
    close(log_pipe->read);

    // Create New Process Group
    setpgid(0, 0);
}
#endif
static void fwrite_with_flush(FILE *stream, const void *data, const size_t size) {
    fwrite(data, size, 1, stream);
    fflush(stream);
}
static int setup_logger_parent(Process &child) {
    // This process will:
    // * Forward the child's output to the terminal and the log file.
    // * Forward the terminal's input to the child.
    // * Display the crash report dialog when needed.

    // Set Debug Tag
    reborn_debug_tag = "(Logger) ";
    DEBUG("Writing To: %s", log->name.c_str());

#ifndef _WIN32
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
#endif

    // Poll
    std::vector fds = {
        child.fds[0],
        child.fds[1]
    };
#ifndef _WIN32
    fds.push_back(log_pipe->read);
#endif
    poll_fds(fds, true, [&child](const int i, const size_t size, unsigned char *buf) {
        switch (i) {
            case 0:
            case 1: {
                // Source: Child's stdout/stderr
                // Action: Print to the terminal
                FILE *target = i == 0 ? stdout : stderr;
                fwrite_with_flush(target, buf, size);
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
                HANDLE fd = child.fds[2];
#ifndef _WIN32
                safe_write(fd, buf, size);
#else
                DWORD bytes_written;
                const BOOL ret = WriteFile(fd, buf, size, &bytes_written, nullptr);
                if (!ret && bytes_written != size) {
                    ERR("Unable To Write To Child's Input");
                }
#endif
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
        fwrite_with_flush(stderr, data, size);
        // Write Exit Code Log Line
        log->write(data, size, true);
    }

    // Close Log File
#ifndef _WIN32
    delete log_pipe;
#endif
    const std::string log_filename = log->name;
    delete log;
    reborn_set_log(-1);

    // Show Crash Log
    if (is_crash && !reborn_is_headless()) {
        show_report(log_filename);
    }

    // Exit
    return
#ifdef _WIN32
        status
#else
        WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE
#endif
        ;
}

// Main
int main(MCPI_UNUSED const int argc, char *argv[]) {
    //AllocConsole();
    //AttachConsole(GetCurrentProcessId());
    //freopen("CON", "w", stdout);
    //freopen("CON", "w", stderr);
    // Setup Logging
    setup_log_file();

    // Fork
#ifndef _WIN32
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
#else
    argv++;
    Process child = spawn_with_stdio(argv);
    return setup_logger_parent(child);
#endif
}
