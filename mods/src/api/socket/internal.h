#pragma once

#include <cstdint>

#include "../internal.h"

// Temporal Budgeting
struct TimeLimiter {
    TimeLimiter();
    void reset();
    [[nodiscard]] bool check() const;
private:
    uint64_t last_time;
};

// Writing
MCPI_INTERNAL void api_clear_write_queue(int sock);
MCPI_INTERNAL void api_clear_write_queue();
MCPI_INTERNAL void api_write_queued_data(int sock);
MCPI_INTERNAL void api_queue_data_to_write(int sock, const std::string &data);

// Reading
MCPI_INTERNAL void api_reset_read_and_handle_budget();
MCPI_INTERNAL bool api_read_and_handle_data(CommandServer *self, ConnectedClient &client);