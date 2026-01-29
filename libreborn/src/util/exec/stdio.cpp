#ifndef _WIN32
#include <sys/wait.h>
#include <sys/prctl.h>
#include <cstring>
#endif

#include <libreborn/util/exec.h>
#include <libreborn/util/io.h>
#include <libreborn/util/logger.h>
#include <libreborn/log.h>

// Process Object
Process::Process(const process_t &pid_, const std::array<HANDLE, fd_count> &fds_):
    open(true),
    pid(pid_),
    fds(fds_) {}
Process::Process(const std::array<HANDLE, fd_count> &fds_):
    open(false),
    pid(),
    fds(fds_)
{
    close_fds();
}

// Close Process
void Process::close_fds() const {
    for (const HANDLE fd : fds) {
        CloseHandle(fd);
    }
}
exit_status_t Process::close() {
    // Check If Process Was Already Closed
    if (!open) {
        int ret = EXIT_FAILURE;
#ifndef _WIN32
        ret = W_EXITCODE(ret, 0);
#endif
        return ret;
    }
    open = false;
    // Close Handles
    close_fds();
    // Wait For Process To Exit
#ifdef _WIN32
    WaitForSingleObject(pid.hProcess, INFINITE);
    DWORD status = EXIT_FAILURE;
    GetExitCodeProcess(pid.hProcess, &status);
    CloseHandle(pid.hProcess);
    CloseHandle(pid.hThread);
#else
    int status = W_EXITCODE(EXIT_FAILURE, 0);
    waitpid(pid, &status, 0);
#endif
    return status;
}

// Spawn Processes
#ifndef _WIN32
std::optional<Process> fork_with_stdio() {
    // Store Output
    const std::array<Pipe, Process::fd_count> pipes;

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
        safe_exec(argv);
    }
    return child.value();
}
#else
Process spawn_with_stdio(const char *const argv[]) {
    // Log
    log_command(argv, "Spawning");

    // Store Output
    const std::array<Pipe, Process::fd_count> pipes;

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
    const bool success = CreateProcessA(
        nullptr,
        (LPSTR) cmd.c_str(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );
    if (!success) {
        WARN("Unable To Spawn Process: %s: %lu", argv[0], GetLastError());
    }

    // Close Unneeded File Descriptors
    CloseHandle(pipes.at(0).write);
    CloseHandle(pipes.at(1).write);

    // Return
    const std::array fds = {pipes.at(0).read, pipes.at(1).read};
    if (success) {
        return Process(pi, fds);
    } else {
        return Process(fds);
    }
}
#endif

// Run Command And Get Output
std::vector<unsigned char> *run_command(const char *const command[], exit_status_t *exit_status) {
    // Run
    Process child = spawn_with_stdio(command);

    // Read stdout
    std::vector<unsigned char> *output = new std::vector<unsigned char>;
    if (child.open) {
        poll_fds({child.fds.at(0), child.fds.at(1)}, {}, [&output](const int i, const size_t size, const unsigned char *buf) {
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
    }

    // Get Exit Status
    const exit_status_t status = child.close();
    if (exit_status != nullptr) {
        *exit_status = status;
    }

    // Add NULL-Terminator To Output
    output->push_back(0);

    // Return
    return output;
}