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
#ifdef _WIN32
std::string make_cmd(const char *const argv[]);
#endif
__attribute__((noreturn)) void safe_execvpe(const char *const argv[]);

// fork() With I/O
#ifdef _WIN32
typedef PROCESS_INFORMATION process_t;
typedef DWORD exit_status_t;
#else
typedef pid_t process_t;
typedef int exit_status_t;
#endif
struct Process {
    static constexpr int fd_count = 2;
    explicit Process(const process_t &pid_, const std::array<HANDLE, fd_count> &fds_);
    explicit Process(const std::array<HANDLE, fd_count> &fds_);
    // Close
    void close_fds() const;
    [[nodiscard]] exit_status_t close();
    // Data
    bool open;
    const process_t pid;
    const std::array<HANDLE, fd_count> fds;
};
#ifndef _WIN32
std::optional<Process> fork_with_stdio();
#endif
Process spawn_with_stdio(const char *const argv[]);

// Easily Poll FDs
void poll_fds(
    std::vector<HANDLE> fds,
    const std::unordered_set<HANDLE> &do_not_expect_to_close,
    const std::function<void(int, size_t, const unsigned char *)> &on_data
);

// Run Command And Get Output
std::vector<unsigned char> *run_command(const char *const command[], exit_status_t *exit_status);
bool is_exit_status_success(exit_status_t status, bool allow_ctrl_c = false);

// Get Exit Status String
std::string get_exit_status_string(exit_status_t status);

// Open URL
void open_url(const std::string &url);

// Download From Internet
const std::vector<unsigned char> *download_from_internet(const std::string &dest, const std::string &url, const std::optional<std::string> &user_agent = std::nullopt);

// WSL Command-Line Options
#ifdef _WIN32
#define WSL_FLAGS "--distribution", "MCPI-Reborn"
#endif