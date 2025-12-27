#include <sys/socket.h>

#include "internal.h"

// Queue Data For Writing
static std::unordered_map<int, std::string> to_write;

// Cleanup Queue
static void cleanup_write_queue() {
    std::erase_if(to_write, [](const std::pair<const int, std::string> &pair) {
        return pair.second.empty();
    });
}

// Clear Queue
void api_clear_write_queue(const int sock) {
    to_write.erase(sock);
    cleanup_write_queue();
}
void api_clear_write_queue() {
    to_write.clear();
}

// Write Queued Data
void api_write_queued_data(const int sock) {
    // Check
    if (!to_write.contains(sock)) {
        return;
    }

    // Write Data
    std::string &data = to_write.at(sock);
    size_t remaining = data.size();
    size_t pos = 0;
    while (remaining > 0) {
        const ssize_t bytes_sent = send(sock, data.data() + pos, remaining, 0);
        if (bytes_sent == -1) {
            // Handle Error
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                // Unable To Send Right Now, Try Again Later
            } else {
                // Unknown Error, Give Up
                pos += remaining;
            }
            remaining = 0;
        } else {
            // Advance
            pos += bytes_sent;
            remaining -= bytes_sent;
        }
    }

    // Erase Sent Data
    data.erase(0, pos);

    // Remove Finished Sockets
    cleanup_write_queue();
}

// Queue Data
void api_queue_data_to_write(const int sock, const std::string &data) {
    to_write[sock] += data;
}