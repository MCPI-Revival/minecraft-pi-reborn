#pragma once

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include "log.h"

// Check Memory Allocation
#define ALLOC_CHECK(obj) \
    { \
        if (obj == NULL) { \
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
#ifdef __cplusplus
#define hooked_function_setup extern "C"
#else
#define hooked_function_setup
#endif
#define HOOK(name, return_type, args) \
    EXTERNAL_FUNC(name, return_type, args) \
    hooked_function_setup __attribute__((__used__)) return_type name args

#ifdef __cplusplus
extern "C" {
#endif

// Safe Version Of pipe()
void safe_pipe2(int pipefd[2], int flags);
// Check If Two Percentages Are Different Enough To Be Logged
int is_progress_difference_significant(int32_t new_val, int32_t old_val);

// Lock File
int lock_file(const char *file);
void unlock_file(const char *file, int fd);

// Access Configuration At Runtime
const char *reborn_get_version();
int reborn_is_headless();
int reborn_is_server();

// Check $DISPLAY
void reborn_check_display();

// Get Home Subdirectory
const char *get_home_subdirectory_for_game_data();

// Make Sure Directory Exists
void ensure_directory(const char *path);

// Customize VTable
#define CUSTOM_VTABLE(name, parent) \
    void _setup_##name##_vtable(parent##_vtable *vtable); \
    static parent##_vtable *get_##name##_vtable() { \
        static parent##_vtable *vtable = NULL; \
        /* Allocate VTable */ \
        if (vtable == NULL) { \
            /* Init */ \
            vtable = dup_vtable(parent##_vtable_base); \
            ALLOC_CHECK(vtable); \
            /* Setup */ \
            _setup_##name##_vtable(vtable); \
        } \
        /* Return */ \
        return vtable; \
    } \
    /* User-Defined Setup Code */ \
    void _setup_##name##_vtable(parent##_vtable *vtable)

#ifdef __cplusplus
}
#endif
