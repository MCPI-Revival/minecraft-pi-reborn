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

// Hook Library Function
#define HOOK(name, return_type, args) \
    typedef return_type (*name##_t)args; \
    static name##_t real_##name = NULL; \
    \
    __attribute__((__unused__)) static void ensure_##name() { \
        if (!real_##name) { \
            dlerror(); \
            real_##name = (name##_t) dlsym(RTLD_NEXT, #name); \
            if (!real_##name) { \
                ERR("Error Resolving Symbol: " #name ": %s", dlerror()); \
            } \
        } \
    } \
    \
    __attribute__((__used__)) return_type name args

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

#ifdef __cplusplus
}
#endif
