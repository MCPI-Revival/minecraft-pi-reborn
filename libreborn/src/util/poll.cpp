#ifndef _WIN32
#include <sys/poll.h>
#include <cstring>
#include <unistd.h>
#endif

#include <libreborn/util/exec.h>
#include <libreborn/log.h>

// Poll FDs
#define BUFFER_SIZE 1024
void poll_fds(std::vector<HANDLE> fds, const std::unordered_set<HANDLE> &do_not_expect_to_close, const std::function<void(int, size_t, unsigned char *)> &on_data) {
#ifdef _WIN32
    // Track Indices
    std::unordered_map<HANDLE, int> handle_to_index;
    for (std::vector<HANDLE>::size_type i = 0; i < fds.size(); i++) {
        handle_to_index[fds.at(i)] = int(i);
    }

    // Poll
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
        for (std::vector<HANDLE>::size_type j = 0; j < fds.size(); j++) {
            // Check For Data Received
            HANDLE fd = fds.at(j);
            const int i = handle_to_index.at(fd);

            // Get Data Available
            DWORD bytes_available = 0;
            BOOL ret = PeekNamedPipe(fd, nullptr, 0, nullptr, &bytes_available, nullptr);
            if (!ret) {
                // Probably Closed
                std::erase(fds, fd);
                continue;
            }

            // Read Data
            if (bytes_available > 0) {
                unsigned char buf[BUFFER_SIZE];
                DWORD bytes_read = 0;
                ret = ReadFile(fd, buf, std::min<DWORD>(BUFFER_SIZE, bytes_available), &bytes_read, nullptr);
                if (ret && bytes_read > 0) {
                    // Callback
                    on_data(i, bytes_read, buf);
                } else {
                    // Mark As Closed
                    std::erase(fds, fd);
                }
            }
        }
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
                    poll_fd.events = 0;
                }
            }
        }
    }
#endif
}