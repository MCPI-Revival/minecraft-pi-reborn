#include <pthread.h>
#include <sys/prctl.h>
#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/poll.h>
#include <cstring>

#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/util/io.h>

// Fork
Process::Process(const pid_t pid_, const std::array<int, fd_count> &fds_): pid(pid_), fds(fds_) {}
int Process::close() const {
    for (const int fd : fds) {
        ::close(fd);
    }
    int status;
    waitpid(pid, &status, 0);
    return status;
}
#define PIPE_READ 0
#define PIPE_WRITE 1
std::optional<Process> fork_with_stdio() {
    // Store Output
    const std::array<Pipe, Process::fd_count> pipes = {};

    // Fork
    const pid_t ret = fork();
    if (ret == -1) {
        ERR("Unable To Fork: %s", strerror(errno));
    } else if (ret == 0) {
        // Child Process

        // Redirect stdio To Pipes
        dup2(pipes[0].write, STDOUT_FILENO);
        dup2(pipes[1].write, STDERR_FILENO);
        dup2(pipes[2].read, STDIN_FILENO);
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
        close(pipes[0].write);
        close(pipes[1].write);
        close(pipes[2].read);

        // Return
        return Process(ret, {pipes[0].read, pipes[1].read, pipes[2].write});
    }
}
#define BUFFER_SIZE 1024
void poll_fds(const std::vector<int> &fds, const std::function<void(int, size_t, unsigned char *)> &on_data) {
    // Track Open FDs
    int open_fds = int(fds.size());

    // Setup Polling
    pollfd *poll_fds = new pollfd[fds.size()];
    for (std::vector<int>::size_type i = 0; i < fds.size(); i++) {
        const int fd = fds[i];
        poll_fds[i].fd = fd;
        poll_fds[i].events = POLLIN;
        if (fd == STDIN_FILENO) {
            // Don't Wait For stdin To Close
            open_fds--;
        }
    }

    // Poll
    while (open_fds > 0) {
        const int poll_ret = poll(poll_fds, fds.size(), -1);
        if (poll_ret == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                ERR("Unable To Poll Data: %s", strerror(errno));
            }
        }

        // Handle Data
        for (std::vector<int>::size_type i = 0; i < fds.size(); i++) {
            pollfd &poll_fd = poll_fds[i];
            if (poll_fd.revents != 0 && poll_fd.events != 0) {
                if (poll_fd.revents & POLLIN) {
                    // Data Available From Child's stdout/stderr
                    unsigned char buf[BUFFER_SIZE];
                    const ssize_t bytes_read = read(poll_fd.fd, buf, BUFFER_SIZE);
                    if (bytes_read == -1) {
                        ERR("Unable To Read Data: %s", strerror(errno));
                    }
                    // Callback
                    on_data(int(i), size_t(bytes_read), buf);
                } else {
                    // File Descriptor No Longer Accessible
                    if (poll_fd.fd == STDIN_FILENO) {
                        // This Shouldn't Happen
                        IMPOSSIBLE();
                    }
                    poll_fd.events = 0;
                    open_fds--;
                }
            }
        }
    }

    // Cleanup
    delete[] poll_fds;
}

// Safe execvpe()
__attribute__((noreturn)) void safe_execvpe(const char *const argv[], const char *const envp[]) {
    // Log
    DEBUG("Running Command:");
    for (int i = 0; argv[i] != nullptr; i++) {
        DEBUG("    %s", argv[i]);
    }
    // Run
    const int ret = execvpe(argv[0], (char *const *) argv, (char *const *) envp);
    if (ret == -1) {
        ERR("Unable To Execute Program: %s: %s", argv[0], strerror(errno));
    } else {
        IMPOSSIBLE();
    }
}

// Run Command And Get Output
#define CHILD_PROCESS_TAG "(Child Process) "
std::vector<unsigned char> *run_command(const char *const command[], int *exit_status) {
    // Run
    const std::optional<Process> child = fork_with_stdio();
    if (!child) {
        // Child Process
        reborn_debug_tag = CHILD_PROCESS_TAG;
        // Run
        safe_execvpe(command, environ);
    } else {
        // Close stdin
        close(child->fds[2]);
        // Read stdout
        std::vector<unsigned char> *output = new std::vector<unsigned char>;
        poll_fds({child->fds[0], child->fds[1]}, [&output](const int i, const size_t size, unsigned char *buf) {
            if (i == 0) {
                // stdout
                output->insert(output->end(), buf, buf + size);
            } else {
                // stderr
                safe_write(reborn_get_debug_fd(), buf, size);
            }
        });

        // Get Exit Status
        const int status = child->close();
        if (exit_status != nullptr) {
            *exit_status = status;
        }

        // Add NULL-Terminator To Output
        output->push_back(0);

        // Return
        return output;
    }
}

// Get Exit Status String
std::string get_exit_status_string(const int status) {
    if (WIFEXITED(status)) {
        return ": Exit Code: " + std::to_string(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        return ": Signal: " + std::to_string(WTERMSIG(status)) + (WCOREDUMP(status) ? " (Core Dumped)" : "");
    } else {
        return "";
    }
}

// Check Exit Status
bool is_exit_status_success(const int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status) == 0;
    } else if (WIFSIGNALED(status)) {
        const int signal_no = WTERMSIG(status);
        return signal_no == SIGINT || signal_no == SIGTERM;
    } else {
        return false;
    }
}

// Open URL
void open_url(const std::string &url) {
    int return_code;
    const char *command[] = {"xdg-open", url.c_str(), nullptr};
    const std::vector<unsigned char> *output = run_command(command, &return_code);
    delete output;
    if (!is_exit_status_success(return_code)) {
        WARN("Unable To Open URL: %s", url.c_str());
    }
}