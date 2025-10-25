#include <cstring>

#ifndef _WIN32
#include <sys/wait.h>
#include <sys/prctl.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/util/io.h>
#include <libreborn/util/string.h>

// Safe execvpe()
#ifdef _WIN32
static std::string quote(const std::string &str) {
    // https://learn.microsoft.com/en-us/archive/blogs/twistylittlepassagesallalike/everyone-quotes-command-line-arguments-the-wrong-way
    if (!str.empty() && str.find_first_of (" \t\n\v\"") == std::string::npos) {
        // No Quoting Needed
        return str;
    }
    constexpr char slash = '\\';
    std::string out;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        int slash_count = 0;
        while (it != str.end() && *it == slash) {
            ++it;
            ++slash_count;
        }
        if (it == str.end()) {
            out.append(slash_count * 2, slash);
        } else {
            if (*it == '"') {
                slash_count *= 2;
            }
            out.append(slash_count, slash);
            out.push_back(*it);
        }
    }
    // Return
    return '\"' + out + '\"';
}
std::string make_cmd(const char *const argv[]) {
    std::string cmd;
    for (int i = 0; argv[i] != nullptr; i++) {
        if (!cmd.empty()) {
            cmd += ' ';
        }
        cmd += quote(argv[i]);
    }
    return cmd;
}
#endif
static void log_command(const char *const argv[], const char *verb = "Running") {
    DEBUG("%s Command:", verb);
    for (int i = 0; argv[i] != nullptr; i++) {
        DEBUG("    %s", argv[i]);
    }
}
void safe_execvpe(const char *const argv[]) {
    // Log
    log_command(argv);

    // Run
#ifndef _WIN32
    const int ret = execvpe(argv[0], (char *const *) argv, environ);
#else
    SetConsoleCtrlHandler(nullptr, TRUE);
    const std::string cmd = make_cmd(argv);
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    const int ret = CreateProcessA(
        nullptr,
        (LPSTR) cmd.c_str(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    ) ? 0 : -1;
#endif

    // Check Result
    if (ret == -1) {
        ERR("Unable To Execute Program: %s: %s", argv[0], strerror(errno));
    } else {
#ifndef _WIN32
        IMPOSSIBLE();
#else
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        exit(int(exitCode));
#endif
    }
}

// Fork
Process::Process(const process_t &pid_, const std::array<HANDLE, fd_count> &fds_):
    pid(pid_),
    fds(fds_) {}
int Process::close() {
    // Close Handles
    for (const HANDLE fd : fds) {
        CloseHandle(fd);
    }
    // Wait For Process To Exit
#ifdef _WIN32
    WaitForSingleObject(pid.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pid.hProcess, &exitCode);
    CloseHandle(pid.hProcess);
    CloseHandle(pid.hThread);
    const int status = int(exitCode);
#else
    int status;
    waitpid(pid, &status, 0);
#endif
    return status;
}

// Spawn Processes
#ifndef _WIN32
#define PIPE_READ 0
#define PIPE_WRITE 1
std::optional<Process> fork_with_stdio() {
    // Store Output
    const std::array<Pipe, Process::fd_count> pipes = {
        Pipe(true),
        Pipe(true)
    };

    // Fork
    const pid_t ret = fork();
    if (ret == -1) {
        ERR("Unable To Fork: %s", strerror(errno));
    } else if (ret == 0) {
        // Child Process

        // Redirect stdio To Pipes
        dup2(pipes.at(0).write, STDOUT_FILENO);
        dup2(pipes.at(1).write, STDERR_FILENO);
        for (const Pipe &pipe : pipes) {
            close(pipe.write);
            close(pipe.read);
        }

        // Kill Child If Parent Exits First
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        // Continue Execution
        return {};
    } else {
        // Parent Process

        // Close Unneeded File Descriptors
        close(pipes.at(0).write);
        close(pipes.at(1).write);

        // Return
        return Process(ret, {pipes.at(0).read, pipes.at(1).read});
    }
}
Process spawn_with_stdio(const char *const argv[]) {
    // Run
    const std::optional<Process> child = fork_with_stdio();
    if (!child) {
        // Child Process
        reborn_debug_tag = DEBUG_TAG("Child Process");
        // Run
        safe_execvpe(argv);
    }
    return child.value();
}
#else
Process spawn_with_stdio(const char *const argv[]) {
    // Log
    log_command(argv, "Spawning");

    // Store Output
    const std::array<Pipe, Process::fd_count> pipes = {
        Pipe(true),
        Pipe(true)
    };

    // Configure Pipes
    for (const HANDLE fd : {pipes.at(0).read, pipes.at(1).read}) {
        if (!SetHandleInformation(fd, HANDLE_FLAG_INHERIT, 0)) {
            ERR("Unable To Disable Pipe Inheritance");
        }
    }

    // Create Process
    const std::string cmd = make_cmd(argv);
    STARTUPINFOA si = {};
    si.hStdOutput = pipes.at(0).write;
    si.hStdError = pipes.at(1).write;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessA(
        nullptr,
        (LPSTR) cmd.c_str(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    )) {
        ERR("Unable To Spawn Process: %s", argv[0]);
    }

    // Close Unneeded File Descriptors
    CloseHandle(pipes.at(0).write);
    CloseHandle(pipes.at(1).write);

    // Return
    return Process(pi, {pipes.at(0).read, pipes.at(1).read});
}
#endif

// Run Command And Get Output
std::vector<unsigned char> *run_command(const char *const command[], int *exit_status) {
    // Run
    Process child = spawn_with_stdio(command);

    // Read stdout
    std::vector<unsigned char> *output = new std::vector<unsigned char>;
    poll_fds({child.fds.at(0), child.fds.at(1)}, {}, [&output](const int i, const size_t size, unsigned char *buf) {
        if (i == 0) {
            // stdout
            output->insert(output->end(), buf, buf + size);
        } else {
            // stderr
            FILE *file = reborn_get_debug_file();
            if (file) {
                fwrite(buf, size, 1, file);
                fflush(file);
            }
        }
    });

    // Get Exit Status
    const int status = child.close();
    if (exit_status != nullptr) {
        *exit_status = status;
    }

    // Add NULL-Terminator To Output
    output->push_back(0);

    // Return
    return output;
}

// Get Exit Status String
#ifdef _WIN32
#define WIFEXITED(x) (true)
#define WEXITSTATUS(x) int(x)
#define WIFSIGNALED(x) (false)
#define WTERMSIG(x) (0)
#define WCOREDUMP(x) (false)
#define SIGINT (1)
#define SIGTERM SIGINT
#endif
std::string get_exit_status_string(const int status) {
    if (WIFEXITED(status)) {
        return ": Exit Code: " + safe_to_string(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        return ": Signal: " + safe_to_string(WTERMSIG(status)) + (WCOREDUMP(status) ? " (Core Dumped)" : "");
    } else {
        return "";
    }
}

// Check Exit Status
bool is_exit_status_success(const int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status) == EXIT_SUCCESS;
    } else if (WIFSIGNALED(status)) {
        const int signal_no = WTERMSIG(status);
        return signal_no == SIGINT || signal_no == SIGTERM;
    } else {
        return false;
    }
}

// Open URL
void open_url(const std::string &url) {
#ifndef _WIN32
    int return_code;
    const char *command[] = {"xdg-open", url.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(command, &return_code);
    delete output;
    if (!is_exit_status_success(return_code)) {
        WARN("Unable To Open URL: %s", url.c_str());
    }
#else
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
#endif
}