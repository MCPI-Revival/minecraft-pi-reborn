#pragma once

#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <dlfcn.h>
#include <array>

#include "log.h"

// Check Memory Allocation
#define ALLOC_CHECK(obj) \
    { \
        if ((obj) == nullptr) { \
            ERR("Memory Allocation Failed"); \
        } \
    }

// Align Number
#define ALIGN_UP(x, alignment) \
    ({ \
        int _align_x = (x); \
        int _align_alignment = (alignment); \
        int _align_diff = _align_x % _align_alignment; \
        if (_align_diff > 0) { \
            _align_x += (_align_alignment - _align_diff); \
        } \
        _align_x; \
    })

// Hook Library Function
#define EXTERNAL_FUNC(name, return_type, args) \
    typedef return_type (*real_##name##_t)args; \
    __attribute__((__unused__)) static real_##name##_t real_##name() { \
        static real_##name##_t func = NULL; \
        if (!func) { \
            dlerror(); \
            func = (real_##name##_t) dlsym(RTLD_NEXT, #name); \
            if (!func) { \
                ERR("Error Resolving Symbol: " #name ": %s", dlerror()); \
            } \
        } \
        return func; \
    }
#define HOOK(name, return_type, args) \
    EXTERNAL_FUNC(name, return_type, args) \
    extern "C" __attribute__((__used__)) return_type name args

// Safe Version Of pipe()
struct Pipe {
    Pipe();
    const int read;
    const int write;
};
// Check If Two Percentages Are Different Enough To Be Logged
bool is_progress_difference_significant(int32_t new_val, int32_t old_val);

// Lock File
int lock_file(const char *file);
void unlock_file(const char *file, int fd);

// Access Configuration At Runtime
const char *reborn_get_version();
bool reborn_is_headless();
bool reborn_is_server();

// Check $DISPLAY
void reborn_check_display();

// Get Home Subdirectory
const char *get_home_subdirectory_for_game_data();

// Make Sure Directory Exists
void ensure_directory(const char *path);

// Safe write()
void safe_write(int fd, const void *buf, size_t size);