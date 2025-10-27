#ifndef _WIN32
#include <sys/poll.h>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#include <libreborn/util/exec.h>
#include <libreborn/log.h>

// Buffer Size
#define BUFFER_SIZE 1024

// Utility Functions
static bool get_bytes_available(const HANDLE fd, int &bytes_available_out) {
#ifdef _WIN32
    DWORD bytes_available = 0;
    const BOOL ret = PeekNamedPipe(fd, nullptr, 0, nullptr, &bytes_available, nullptr);
    if (!ret) {
        return false;
    }
    bytes_available_out = int(bytes_available);
#else
    int available_bytes = 0;
    const int ret = ioctl(fd, FIONREAD, &available_bytes);
    if (ret != 0) {
        return false;
    }
    bytes_available_out = available_bytes;
#endif
    return true;
}
static bool read_fd(const HANDLE fd, unsigned char *buf, const size_t size, size_t &bytes_read_out) {
#ifdef _WIN32
    DWORD bytes_read = 0;
    const bool ret = ReadFile(fd, buf, size, &bytes_read, nullptr);
    if (!ret) {
        return false;
    }
#else
    const ssize_t bytes_read = read(fd, buf, size);
    if (bytes_read < 0) {
        return false;
    }
#endif
    bytes_read_out = size_t(bytes_read);
    return true;
}

// Poll Single FD
static bool poll_fd(const int i, const HANDLE fd, unsigned char buf[BUFFER_SIZE], const std::function<void(int, size_t, const unsigned char *)> &on_data) {
    while (true) {
        // Get Data Available
        int bytes_available = 0;
        bool ret = get_bytes_available(fd, bytes_available);
        if (!ret) {
            // Probably Closed
            return false;
        }
        if (bytes_available <= 0) {
            // No Data Available
            return true;
        }

        // Read Data
        const size_t bytes_to_read = std::min<size_t>(BUFFER_SIZE, bytes_available);
        size_t bytes_read = 0;
        ret = read_fd(fd, buf, bytes_to_read, bytes_read);
        if (!ret) {
            // Read Failure
            // Mark As Closed
            return false;
        }
        if (bytes_read > 0) {
            // Successfully Read Data
            on_data(i, bytes_read, buf);
        }
    }
}
#ifndef _WIN32
static bool poll_fd(const int i, const pollfd &fd, unsigned char buf[BUFFER_SIZE], const std::function<void(int, size_t, const unsigned char *)> &on_data) {
    // Check poll() Result
    if (fd.revents == 0 || fd.events == 0) {
        // No Data Available
        return true;
    }
    if (!(fd.revents & POLLIN)) {
        // File Descriptor No Longer Accessible
        return false;
    }
    // Read Data
    return poll_fd(i, fd.fd, buf, on_data);
}
#endif

// Poll FDs
void poll_fds(std::vector<HANDLE> fds, const std::unordered_set<HANDLE> &do_not_expect_to_close, const std::function<void(int, size_t, const unsigned char *)> &on_data) {
#ifdef _WIN32
    // Track Indices
    std::unordered_map<HANDLE, int> handle_to_index;
    for (std::vector<HANDLE>::size_type i = 0; i < fds.size(); i++) {
        handle_to_index[fds.at(i)] = int(i);
    }

    // Detect Console Inputs
    std::vector<bool> is_console;
    for (const HANDLE &fd : fds) {
        DWORD mode;
        is_console.push_back(GetConsoleMode(fd, &mode));
    }

    // Poll
    unsigned char buf[BUFFER_SIZE];
    while (true) {
        // Count Open Pipes
        int open_fds = 0;
        for (const HANDLE &fd : fds) {
            if (!do_not_expect_to_close.contains(fd)) {
                open_fds++;
            }
        }
        if (open_fds <= 0) {
            break;
        }

        // Wait For Data
        // This is a really inefficient solution,
        // but Windows just doesn't have a nice way
        // to wait for pipes to be readable.
        std::vector<HANDLE> to_remove;
        for (const HANDLE fd : fds) {
            // Get Index
            const int i = handle_to_index.at(fd);

            // Read Data (If Available)
            const bool ret = poll_fd(i, fd, buf, on_data);
            if (!ret) {
                // Closed
                to_remove.push_back(fd);
            }
        }

        // Remove Closed FDs
        for (const HANDLE fd : to_remove) {
            std::erase(fds, fd);
        }

        // Prevent 100% CPU
        Sleep(10);
    }
#else
    // Setup Polling
    std::vector<pollfd> poll_fds;
    for (const int fd : fds) {
        pollfd poll_fd = {};
        poll_fd.fd = fd;
        poll_fd.events = POLLIN;
        poll_fds.push_back(poll_fd);
    }

    // Poll
    unsigned char buf[BUFFER_SIZE];
    while (true) {
        // Count Open FDs
        int open_fds = 0;
        for (const pollfd &fd : poll_fds) {
            if (fd.events != 0 && !do_not_expect_to_close.contains(fd.fd)) {
                open_fds++;
            }
        }
        if (open_fds <= 0) {
            break;
        }

        // Wait For Data
        const int poll_ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_ret == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                ERR("Unable To Poll Data: %s", strerror(errno));
            }
        }

        // Handle Data
        for (std::vector<int>::size_type i = 0; i < fds.size(); i++) {
            pollfd &fd = poll_fds.at(i);
            const bool ret = poll_fd(int(i), fd, buf, on_data);
            if (!ret) {
                // Closed
                fd.events = 0;
            }
        }
    }
#endif
}