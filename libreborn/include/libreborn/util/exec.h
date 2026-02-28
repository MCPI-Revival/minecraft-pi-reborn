#pragma once

#include <string>
#include <optional>
#include <array>
#include <vector>
#include <functional>
#include <unordered_set>

#include "io.h"

// Log A Command
#define DEFAULT_LOG_COMMAND_VERB "Running"
MCPI_REBORN_UTIL_PUBLIC void log_command(const char *const argv[], const char *verb = DEFAULT_LOG_COMMAND_VERB);

// Safe execvpe()
// On Windows, this spawns a child process.
#ifdef _WIN32
MCPI_REBORN_UTIL_PUBLIC std::string make_cmd(const char *const argv[]);
#endif
MCPI_REBORN_UTIL_PUBLIC __attribute__((noreturn)) void safe_exec(const char *const argv[]);

// fork() With I/O
#ifdef _WIN32
typedef PROCESS_INFORMATION process_t;
typedef DWORD exit_status_t;
#else
typedef pid_t process_t;
typedef int exit_status_t;
#endif
struct MCPI_REBORN_UTIL_PUBLIC Process {
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
MCPI_REBORN_UTIL_PUBLIC std::optional<Process> fork_with_stdio();
#endif
MCPI_REBORN_UTIL_PUBLIC Process spawn_with_stdio(const char *const argv[]);

// Easily Poll FDs
MCPI_REBORN_UTIL_PUBLIC void poll_fds(
    std::vector<HANDLE> fds,
    const std::unordered_set<HANDLE> &do_not_expect_to_close,
    const std::function<void(int, size_t, const unsigned char *)> &on_data
);

// Run Command And Get Output
MCPI_REBORN_UTIL_PUBLIC std::vector<unsigned char> *run_command(const char *const command[], exit_status_t *exit_status);
MCPI_REBORN_UTIL_PUBLIC bool is_exit_status_success(exit_status_t status, bool allow_ctrl_c = false);

// Get Exit Status String
MCPI_REBORN_UTIL_PUBLIC std::string get_exit_status_string(exit_status_t status);

// Open URL
MCPI_REBORN_UTIL_PUBLIC void open_url(const std::string &url);

// Download From Internet
MCPI_REBORN_UTIL_PUBLIC const std::vector<unsigned char> *download_from_internet(const std::string &dest, const std::string &url, const std::optional<std::string> &user_agent = std::nullopt);

// WSL Command-Line Options
#ifdef _WIN32
#define WSL_FLAGS "--distribution", "MCPI-Reborn"
#endif