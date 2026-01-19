#include <sys/socket.h>
#include <libreborn/log.h>

#include <mods/misc/misc.h>

#include "internal.h"

// Handle Command
static void handle_line(CommandServer *self, ConnectedClient &client, const std::string &line) {
    // Run
    const std::string ret = self->parse(client, line);
    // Return
    if (ret != CommandServer::NullString) {
        if (ret.back() != '\n') {
            IMPOSSIBLE();
        }
        // Queue For Sending
        api_queue_data_to_write(client.sock, ret);
    }
}

// Buffer Overview:
// - Read Buffer:
//   - Used for reading data from the socket.
//   - This has a fixed (small) size.
// - Execute Buffer:
//   - This stores commands that have been read but not yet executed.
//   - Stored in ConnectedClient::str.
//   - This can dynamically grow, but *should* be small.
static constexpr int READ_BUFFER_SIZE = 256;
static constexpr int EXECUTE_BUFFER_TOO_BIG_SIZE = 2048; // 2 KiB

// Locate Command In Execute Buffer
static size_t locate_command_end(ConnectedClient &client) {
    size_t end = client.str.find('\n');
    if (end != std::string::npos) {
        end++;
    }
    return end;
}

// Read Data Into Buffer
enum class ReadResult {
    CLOSED,
    DATA,
    NO_DATA
};
static ReadResult read_data(ConnectedClient &client) {
    // Prevent Massive Execute Buffer
    const bool is_command_available = locate_command_end(client) != std::string::npos;
    const bool is_execute_buffer_too_big = client.str.size() > EXECUTE_BUFFER_TOO_BIG_SIZE;
    // This explicitly disables the size limit
    // when no command is available yet to
    // allow super long commands.
    if (is_command_available && is_execute_buffer_too_big) {
        // A command is already available to execute,
        // and the execute buffer is too big,
        // so skip reading from the socket for now.
        return ReadResult::NO_DATA;
    }

    // Read Buffer
    static char buffer[READ_BUFFER_SIZE];

    // Read Data
    const ssize_t bytes_received = recv(client.sock, buffer, READ_BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
            // End Of Available Data (For Now)
            return ReadResult::NO_DATA;
        } else {
            // Error
            return ReadResult::CLOSED;
        }
    } else if (bytes_received == 0) {
        // Connection Closed
        return ReadResult::CLOSED;
    }

    // Append Received Data To Execute Buffer
    client.str.append(buffer, bytes_received);
    return ReadResult::DATA;
}

// Budget
static TimeLimiter budget;

// Handle Lines Inside The Execute Buffer
static void handle_lines(CommandServer *self, ConnectedClient &client) {
    while (true) {
        // Locate Line
        const size_t end = locate_command_end(client);
        if (end == std::string::npos) {
            break;
        }

        // Extract Line
        std::string line;
        line.assign(client.str.data(), end);
        client.str.erase(0, end);

        // Process
        handle_line(self, client, line);

        // Check Budget
        if (!budget.check()) {
            // Out Of Time!
            break;
        }
    }
}

// Read & Handle Data
void api_reset_read_and_handle_budget() {
    // Reset Time Budget
    budget.reset();
}
bool api_read_and_handle_data(CommandServer *self, ConnectedClient &client) {
    // Read & Handle Lines
    bool closed = false;
    bool more_data_to_read = true;
    while (true) {
        // Read Data
        if (!closed) {
            const ReadResult ret = read_data(client);
            switch (ret) {
                case ReadResult::CLOSED: closed = true; [[fallthrough]];
                case ReadResult::NO_DATA: more_data_to_read = false; break;
                case ReadResult::DATA: break;
            }
        }

        // Process Lines
        handle_lines(self, client);

        // Check
        if (!more_data_to_read || !budget.check()) {
            // Out Of Time/Data!
            break;
        }
    }

    // Finish
    if (closed) {
        // Other End Of Socket Has Closed

        // Check if all commands have been executed.
        const size_t end = locate_command_end(client);
        const bool all_commands_executed = end == std::string::npos;

        // Only close the socket if all
        // commands have been processed.
        if (all_commands_executed) {
            // Close Socket Entirely
            if (!client.str.empty()) {
                const std::string base64 = misc_base64_encode(client.str);
                WARN("Incomplete Data Left In API Socket: %s", base64.c_str());
            }
            return false;
        } else {
            // Keep Socket Open
            return true;
        }
    } else {
        // Socket Is Still Open

        // Write Queued Data
        api_write_queued_data(client.sock);

        // Keep Socket Open
        return true;
    }
}