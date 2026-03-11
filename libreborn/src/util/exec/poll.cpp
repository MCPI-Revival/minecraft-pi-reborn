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
    const BOOL peek_ret = PeekNamedPipe(fd, nullptr, 0, nullptr, &bytes_available, nullptr);
    if (peek_ret) {
        bytes_available_out = int(bytes_available);
        return true;
    }
#else
    int available_bytes = 0;
    const int ret = ioctl(fd, FIONREAD, &available_bytes);
    if (ret == 0) {
        bytes_available_out = available_bytes;
        return true;
    }
#endif
    return false;
}
static bool read_fd(const HANDLE fd, unsigned char *buf, const size_t size, size_t &bytes_read_out) {
    bool ret = true;
#ifdef _WIN32
    DWORD bytes_read;
    const bool read_ret = ReadFile(fd, buf, size, &bytes_read, nullptr);
    if (!read_ret) {
        ret = false;
    }
#else
    ssize_t bytes_read = read(fd, buf, size);
    if (bytes_read <= 0) {
        bytes_read = 0;
        ret = false;
    }
#endif
    bytes_read_out = size_t(bytes_read);
    return ret;
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
        size_t total_bytes_read = 0;
        bool failure = false;
        while (total_bytes_read < bytes_to_read) {
            // Read Chunk
            size_t bytes_read;
            ret = read_fd(fd, buf + total_bytes_read, bytes_to_read - total_bytes_read, bytes_read);
            total_bytes_read += bytes_read;
            if (!ret) {
                // Read Failure
                // Mark As Closed
                failure = true;
                break;
            }
        }
        if (total_bytes_read > 0) {
            // Successfully Read Data
            on_data(i, total_bytes_read, buf);
        }
        if (failure) {
            // Assume FD Is Closed
            return false;
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
    std::unordered_map<HANDLE, int> handle_to_original_index;
    for (std::vector<HANDLE>::size_type i = 0; i < fds.size(); i++) {
        handle_to_original_index.insert({fds.at(i), int(i)});
    }

    // Poll
    unsigned char buf[BUFFER_SIZE];
    while (true) {
        // Wait For Data
        // This is a really inefficient solution,
        // but Windows just doesn't have a nice way
        // to wait for pipes to be readable.
        std::vector<HANDLE> to_remove;
        for (const HANDLE fd : fds) {
            // Get Index
            const int i = handle_to_original_index.at(fd);

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
        for (std::vector<HANDLE>::size_type i = 0; i < fds.size(); i++) {
            pollfd &fd = poll_fds.at(i);
            const bool ret = poll_fd(int(i), fd, buf, on_data);
            if (!ret) {
                // Closed
                fd.events = 0;
            }
        }

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
    }
#endif
}