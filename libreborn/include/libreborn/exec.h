#pragma once

#include <string>
#include <optional>
#include <array>
#include <vector>
#include <functional>

// fork() With I/O
struct Process {
    static constexpr int fd_count = 3;
    Process(pid_t pid_, std::array<int, fd_count> fds_);
    [[nodiscard]] int close() const;
    const pid_t pid;
    const std::array<int, fd_count> fds;
};
std::optional<Process> fork_with_stdio();
void poll_fds(const std::vector<int> &fds, const std::function<void(int, size_t, unsigned char *)> &on_data);

// Safe execvpe()
__attribute__((noreturn)) void safe_execvpe(const char *const argv[], const char *const envp[]);

// Debug Tag
#define CHILD_PROCESS_TAG "(Child Process) "

// Run Command And Get Output
std::vector<unsigned char> *run_command(const char *const command[], int *exit_status);
bool is_exit_status_success(int status);

// Get Exit Status String
std::string get_exit_status_string(int status);

// Open URL
void open_url(const std::string &url);