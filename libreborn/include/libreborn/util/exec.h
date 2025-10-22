#pragma once

#include <string>
#include <optional>
#include <array>
#include <vector>
#include <functional>
#include <unordered_set>

#include "io.h"

// Safe execvpe()
// On Windows, this spawns a child process.
__attribute__((noreturn)) void safe_execvpe(const char *const argv[]);

// fork() With I/O
#ifdef _WIN32
typedef PROCESS_INFORMATION process_t;
#else
typedef pid_t process_t;
#endif
struct Process {
    static constexpr int fd_count = 3;
    Process(const process_t &pid_, const std::array<HANDLE, fd_count> &fds_);
    // Close
    void close_fd(int i);
    [[nodiscard]] int close();
    // Data
    const process_t pid;
    const std::array<HANDLE, fd_count> fds;
    std::unordered_set<int> closed;
};
#ifndef _WIN32
std::optional<Process> fork_with_stdio();
#endif
Process spawn_with_stdio(const char *const argv[]);

// Easily Poll FDs
void poll_fds(std::vector<HANDLE> fds, bool include_stdin, const std::function<void(int, size_t, unsigned char *)> &on_data);

// Run Command And Get Output
std::vector<unsigned char> *run_command(const char *const command[], int *exit_status);
bool is_exit_status_success(int status);

// Get Exit Status String
std::string get_exit_status_string(int status);

// Open URL
void open_url(const std::string &url);