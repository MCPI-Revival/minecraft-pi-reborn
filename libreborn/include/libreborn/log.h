#pragma once

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

// Log File
int reborn_get_log_fd();
void reborn_set_log(int fd);
// Debug Logging
extern const char *reborn_debug_tag;
int reborn_get_debug_fd();

// Logging
#define INFO(format, ...) fprintf(stderr, "[INFO]: " format "\n", ##__VA_ARGS__)
#define WARN(format, ...) fprintf(stderr, "[WARN]: " format "\n", ##__VA_ARGS__)
#define DEBUG(format, ...) dprintf(reborn_get_debug_fd(), "[DEBUG]: %s" format "\n", reborn_debug_tag, ##__VA_ARGS__)
#define ERR(format, ...) \
    ({ \
        fprintf(stderr, "[ERR]: (%s:%i): " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        _exit(EXIT_FAILURE); \
    })
#define IMPOSSIBLE() ERR("This Should Never Be Called")
#define CONDITIONAL_ERR(is_error, ...) \
    ({ \
        if ((is_error)) { \
            ERR(__VA_ARGS__); \
        } else { \
            WARN(__VA_ARGS__); \
        } \
    })
