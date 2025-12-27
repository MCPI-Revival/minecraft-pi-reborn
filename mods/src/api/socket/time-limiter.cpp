#include <ctime>

#include "internal.h"

// Temporal Budget For API Handling
static constexpr uint64_t BUDGET = 10; // In Milliseconds

// Unit Conversion
static constexpr uint64_t NANOSECONDS_PER_MILLISECOND = 1000000;
static constexpr uint64_t MILLISECONDS_PER_SECOND = 1000;

// Get Time
static uint64_t get_time() {
    timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    const uint64_t a = uint64_t(ts.tv_sec) * MILLISECONDS_PER_SECOND;
    const uint64_t b = uint64_t(ts.tv_nsec) / NANOSECONDS_PER_MILLISECOND;
    return a + b;
}

// Handle Budgeting
TimeLimiter::TimeLimiter():
    last_time(0) {}
void TimeLimiter::reset() {
    last_time = get_time();
}
bool TimeLimiter::check() const {
    const uint64_t now = get_time();
    if (now < last_time) {
        return false;
    }
    const uint64_t diff = now - last_time;
    return diff <= BUDGET;
}