#pragma once

#include <cstdio>

// Log File
FILE *reborn_get_log_fd();
void reborn_set_log(int fd);
// Debug Logging
extern const char *reborn_debug_tag;
FILE *reborn_get_debug_fd();

// Logging
#define ATTR __attribute__((format(printf, 1, 2)))
ATTR void INFO(const char *format, ...);
ATTR void WARN(const char *format, ...);
ATTR void DEBUG(const char *format, ...);
ATTR __attribute__((noreturn)) void _ERR(const char *format, ...);
#define ERR(format, ...) _ERR("(%s:%i): " format, __FILE__, __LINE__, ##__VA_ARGS__)
#undef ATTR
#define IMPOSSIBLE() ERR("This Should Never Be Called")
#define CONDITIONAL_ERR(is_error, ...) \
    ({ \
        if ((is_error)) { \
            ERR(__VA_ARGS__); \
        } else { \
            WARN(__VA_ARGS__); \
        } \
    })

// Crash Message
// This is not thread-safe.
const char *reborn_get_crash_message(const char *reason);