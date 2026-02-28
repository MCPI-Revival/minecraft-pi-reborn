#pragma once

#include "../internal.h"

#include <cstdint>

// Temporal Budgeting
struct TimeLimiter {
    TimeLimiter();
    void reset();
    [[nodiscard]] bool check() const;
private:
    uint64_t last_time;
};

// Writing
void api_clear_write_queue(int sock);
void api_clear_write_queue();
void api_write_queued_data(int sock);
void api_queue_data_to_write(int sock, const std::string &data);

// Reading
void api_reset_read_and_handle_budget();
struct CommandServer;
struct ConnectedClient;
bool api_read_and_handle_data(CommandServer *self, ConnectedClient &client);