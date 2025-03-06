#include <sys/socket.h>
#include <unordered_map>

#include <libreborn/patch.h>
#include <libreborn/log.h>

#include <mods/feature/feature.h>

#include "internal.h"

// Queue Data For Writing
static std::unordered_map<int, std::string> to_write;
static void write_queued_data(const int sock) {
    // Check
    if (!to_write.contains(sock)) {
        return;
    }
    // Write Data
    std::string &data = to_write.at(sock);
    size_t pos = 0;
    while (pos < data.size()) {
        const size_t remaining = data.size() - pos;
        ssize_t bytes_sent = send(sock, data.data() + pos, remaining, 0);
        if (bytes_sent == -1) {
            bytes_sent = 0;
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                // Unable To Send Right Now, Try Again Later
                break;
            } else {
                // Error, Give Up
                pos = 0;
                data.clear();
            }
        }
        // Advance
        pos += bytes_sent;
    }
    // Erase Sent Data
    data.erase(0, pos);
    // Remove Finished Sockets
    std::erase_if(to_write, [](const std::pair<const int, std::string> &pair) {
        return pair.second.empty();
    });
}
static void handle_line(CommandServer *self, ConnectedClient &client, const std::string &line) {
    // Run
    const std::string ret = self->parse(client, line);
    // Return
    if (ret != CommandServer::NullString) {
        if (ret.back() != '\n') {
            IMPOSSIBLE();
        }
        // Queue For Sending
        to_write[client.sock] += ret;
    }
}
// Remove Speed Limit
static constexpr int max_read_per_tick_per_client = 16384; // 16 KiB
static bool CommandServer__updateClient_injection_2(__attribute__((unused)) CommandServer__updateClient_t original, CommandServer *self, ConnectedClient &client) {
    // Read Lines
    size_t total_read = 0;
    constexpr size_t buffer_size = 2048;
    char buffer[buffer_size];
    while (total_read < max_read_per_tick_per_client) {
        const ssize_t bytes_received = recv(client.sock, buffer, buffer_size, 0);
        if (bytes_received == -1) {
            if (errno == EINTR) {
                // Signal Detected, Try Again
                continue;
            } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // End Of Available Data (For Now)
                break;
            } else {
                // Error
                return false;
            }
        } else if (bytes_received == 0) {
            // Connection Closed
            return false;
        }
        total_read += bytes_received;
        // Append Received Data To Client Buffer
        client.str.append(buffer, bytes_received);
    }

    // Process Lines
    size_t start = 0;
    size_t end;
    while ((end = client.str.find('\n', start)) != std::string::npos) {
        end++; // Include Newline
        const std::string line = client.str.substr(start, end - start);
        handle_line(self, client, line);
        start = end;
    }
    client.str.erase(0, start);

    // Write Queued Data
    write_queued_data(client.sock);

    // Success
    return true;
}

// Clear Extra Data On Socket Close
static bool CommandServer__updateClient_injection_3(CommandServer__updateClient_t original, CommandServer *self, ConnectedClient &client) {
    const bool ret = original(self, client);
    if (!ret) {
        // Client Disconnected
        api_free_event_data(client.sock);
        to_write.erase(client.sock);
    }
    return ret;
}
static void CommandServer__close_injection_2(CommandServer__close_t original, CommandServer *self) {
    // Clear All Extra Data
    api_free_all_event_data();
    to_write.clear();
    // Call Original Method
    original(self);
}

// Init
void _init_api_socket() {
    // Optimization
    if (feature_has("Optimized API Sockets", server_enabled)) {
        overwrite_calls(CommandServer__updateClient, CommandServer__updateClient_injection_2);
    }
    // Clear Extra Data On Socket Close
    overwrite_calls(CommandServer__updateClient, CommandServer__updateClient_injection_3);
    overwrite_calls(CommandServer__close, CommandServer__close_injection_2);
}