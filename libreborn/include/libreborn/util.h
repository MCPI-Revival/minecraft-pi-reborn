#pragma once

#include <dlfcn.h>
#include <string>

#include "log.h"

// Align Number
int align_up(int x, int alignment);

// Hook Library Function
#define HOOK(name, return_type, args) \
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
    } \
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

// Check $DISPLAY
void reborn_check_display();

// Get Home Subdirectory
const char *get_home_subdirectory_for_game_data();

// Make Sure Directory Exists
void ensure_directory(const char *path);

// Safe write()
void safe_write(int fd, const void *buf, size_t size);

// embed_resource()
#define EMBEDDED_RESOURCE(name) \
    extern unsigned char name[]; \
    extern size_t name##_len

// Profile Directory
std::string home_get();