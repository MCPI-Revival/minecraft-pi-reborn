#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <string>

#ifndef _WIN32
#include <csignal>
#else
#include <fcntl.h>
#endif

#include <libreborn/util/exec.h>
#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/config.h>

#include "writer.h"
#include "../util/util.h"

// Log File
static std::string get_logs_folder() {
    const std::string home = home_get();
    ensure_directory(home.c_str());
    const std::string logs = home + path_separator + "logs";
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
    log_pipe = new Pipe(false);
    const HANDLE log_handle = log_pipe->write;
    reborn_init_log(
#ifdef _WIN32
        _open_osfhandle(intptr_t(log_handle), _O_WRONLY | _O_APPEND)
#else
        log_handle
#endif
    );

    // Create Symlink To Latest Log
#ifndef _WIN32
    const std::string latest_log = logs + path_separator + "latest.log";
    unlink(latest_log.c_str());
    if (symlink(log->name.c_str(), latest_log.c_str()) != 0) {
        WARN("Unable To Create Latest Log Symlink: %s", strerror(errno));
    }
#endif

    // Log
    DEBUG("Writing To: %s", log->name.c_str());
}

// Show Crash Report
static void show_report(const std::string &filename) {
    const std::string exe = get_binary_directory() + path_separator + "crash-report";
    const std::string dir = get_logs_folder();
    const char *argv[] = {exe.c_str(), filename.c_str(), dir.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(argv, nullptr);
    delete output;
}

// Utility Functions
static void fwrite_with_flush(FILE *stream, const void *data, const size_t size) {
    fwrite(data, size, 1, stream);
    fflush(stream);
}

// Setup
#ifndef _WIN32
static void setup_logger_child() {
    // This process will run the actual program.

    // Close Unneeded FD
    close(log_pipe->read);
}
#endif
static int setup_logger_parent(Process &child) {
    // This process will:
    // * Forward the child's output to the terminal and the log file.
    // * Forward the terminal's input to the child.
    // * Display the crash report dialog when needed.

    // Ignore Signals
    // The terminal will automatically pass them
    // to the child process.
#ifndef _WIN32
    for (const int signal_id : {SIGINT, SIGTERM}) {
        signal(signal_id, SIG_IGN);
    }
#else
    SetConsoleCtrlHandler(nullptr, TRUE);
#endif

    // Get Pipes
    const std::vector fds = {
        // Ranked In Order Of Priority
        log_pipe->read, // Debug Log
        child.fds.at(1), // stderr
        child.fds.at(0) // stdout
    };
    const std::unordered_set do_not_expect_to_close = {
        log_pipe->read
    };

    // Poll
    if (child.open) {
        poll_fds(fds, do_not_expect_to_close, [](const int i, const size_t size, const unsigned char *buf) {
            switch (i) {
                case 1:
                case 2: {
                    // Source: Child's stdout/stderr
                    // Action: Print to the terminal
                    FILE *target = i == 1 ? stdout : stderr;
                    fwrite_with_flush(target, buf, size);
                    [[fallthrough]];
                }
                case 0: {
                    // Source: Child's debug log
                    // Action: Write to the log file
                    log->write(buf, std::streamsize(size), false);
                    break;
                }
                default: {}
            }
        });
    }

    // Close Debug Log
    reborn_init_log(-1); // This also closes log_pipe->write.
    CloseHandle(log_pipe->read);
    delete log_pipe;

    // Get Exit Status
    const exit_status_t status = child.close();
    const bool is_crash = !is_exit_status_success(status, true);

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
    const std::string log_filename = log->name;
    delete log;

    // Show Crash Log
    if (is_crash && !reborn_is_headless()) {
        show_report(log_filename);
    }

    // Exit
    return
#ifdef _WIN32
        int(status)
#else
        WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE
#endif
        ;
}

// Main
int main(MCPI_UNUSED const int argc, char *argv[]) {
    // Set Debug Tag
    reborn_debug_tag = DEBUG_TAG("Logger");

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