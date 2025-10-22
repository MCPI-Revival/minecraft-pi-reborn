#ifndef _WIN32
#include <sys/poll.h>
#include <cstring>
#include <unistd.h>
#endif

#include <libreborn/util/exec.h>
#include <libreborn/log.h>

// Poll FDs
#define BUFFER_SIZE 1024
void poll_fds(std::vector<HANDLE> fds, const bool include_stdin, const std::function<void(int, size_t, unsigned char *)> &on_data) {
#ifdef _WIN32
    // Add STDIN
    unsigned int minimum_open_pipes = 1;
    if (include_stdin) {
        const HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
        if (handle && handle != INVALID_HANDLE_VALUE) {
            fds.push_back(handle);
            minimum_open_pipes++;
        }
    }

    // Track Indices
    std::unordered_map<HANDLE, int> handle_to_index;
    for (std::vector<HANDLE>::size_type i = 0; i < fds.size(); i++) {
        handle_to_index[fds.at(i)] = int(i);
    }

    // Poll
    while (fds.size() >= minimum_open_pipes) {
        const DWORD n = fds.size();
        const DWORD wait = WaitForMultipleObjects(n, fds.data(), FALSE, INFINITE);
        if (wait == WAIT_FAILED) {
            // Error
            ERR("Unable To Wait For Data");
        } else if (wait >= WAIT_OBJECT_0 && wait < (WAIT_OBJECT_0 + n)) {
            // Data Received
            const int j = int(wait - WAIT_OBJECT_0);
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
    // Track Open FDs
    int open_fds = int(fds.size());

    // Add STDIN
    if (include_stdin) {
        fds.push_back(STDIN_FILENO);
    }

    // Setup Polling
    pollfd *poll_fds = new pollfd[fds.size()];
    for (std::vector<int>::size_type i = 0; i < fds.size(); i++) {
        const int fd = fds[i];
        poll_fds[i].fd = fd;
        poll_fds[i].events = POLLIN;
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
#endif
}