#include <cstring>

#ifndef _WIN32
#include <sys/wait.h>
#endif

#include <libreborn/log.h>
#include <libreborn/util/exec.h>
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
void log_command(const char *const argv[], const char *verb) {
    DEBUG("%s Command:", verb);
    for (int i = 0; argv[i] != nullptr; i++) {
        DEBUG("    %s", argv[i]);
    }
}
void safe_exec(const char *const argv[]) {
    // Log
    log_command(argv);

    // Run
#ifndef _WIN32
    const char *exe = argv[0];
    const bool success = exe ? execvp(exe, (char *const *) argv) != -1 : false;
    const std::string error = strerror(exe ? errno : ENOENT);
#else
    SetConsoleCtrlHandler(nullptr, TRUE);
    const std::string cmd = make_cmd(argv);
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    const bool success = CreateProcessA(
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
    );
    const std::string error = std::to_string(GetLastError());
#endif

    // Check Result
    if (!success) {
        ERR("Unable To Execute Program: %s: %s", argv[0], error.c_str());
    } else {
#ifndef _WIN32
        IMPOSSIBLE();
#else
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = EXIT_FAILURE;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        exit(int(exitCode));
#endif
    }
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
std::string get_exit_status_string(const exit_status_t status) {
    if (WIFEXITED(status)) {
        return ": Exit Code: " + safe_to_string(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        return ": Signal: " + safe_to_string(WTERMSIG(status)) + (WCOREDUMP(status) ? " (Core Dumped)" : "");
    } else {
        return "";
    }
}

// Check Exit Status
bool is_exit_status_success(const exit_status_t status, const bool allow_ctrl_c) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status) == EXIT_SUCCESS;
    } else if (allow_ctrl_c && WIFSIGNALED(status)) {
        const int signal_no = WTERMSIG(status);
        return signal_no == SIGINT || signal_no == SIGTERM;
    } else {
        return false;
    }
}