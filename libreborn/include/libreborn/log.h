#pragma once

// Logging
#define ATTR __attribute__((format(printf, 1, 2)))
ATTR void INFO(const char *format, ...);
ATTR void WARN(const char *format, ...);
ATTR void _DEBUG(const char *format, ...);
#define DEBUG(format, ...) _DEBUG("%s" format, reborn_debug_tag, ##__VA_ARGS__)
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

// Debug Logging
#define DEBUG_TAG(x) "(" x ") "
extern const char *reborn_debug_tag;