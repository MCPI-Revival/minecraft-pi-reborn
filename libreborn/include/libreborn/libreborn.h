#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

// Logging
#define INFO(msg, ...) fprintf(stderr, "[INFO]: " msg "\n", __VA_ARGS__);
#define ERR(msg, ...) fprintf(stderr, "[ERR]: " msg "\n", __VA_ARGS__); exit(EXIT_FAILURE);

// Check Memory Allocation
#define ALLOC_CHECK(obj) if (obj == NULL) { ERR("(%s:%i) Memory Allocation Failed", __FILE__, __LINE__); }

// Set obj To NULL On asprintf() Failure
#define asprintf(obj, ...) if (asprintf(obj, __VA_ARGS__) == -1) { *obj = NULL; }

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
                ERR("Error Resolving Symbol: "#name": %s", dlerror()); \
            } \
        } \
    }; \
    \
    __attribute__((__used__)) return_type name args

// Sanitize String
void sanitize_string(char **str, int max_length, unsigned int allow_newlines);

// Patching Functions

void _overwrite_call(const char *file, int line, void *start, void *target);
#define overwrite_call(start, target) _overwrite_call(__FILE__, __LINE__, start, target);

void _overwrite_calls(const char *file, int line, void *start, void *target);
#define overwrite_calls(start, target) _overwrite_calls(__FILE__, __LINE__, start, target);

void _overwrite(const char *file, int line, void *start, void *target);
#define overwrite(start, target) _overwrite(__FILE__, __LINE__, start, target);

void _patch(const char *file, int line, void *start, unsigned char patch[]);
#define patch(start, patch) _patch(__FILE__, __LINE__, start, patch);

void _patch_address(const char *file, int line, void *start, void *target);
#define patch_address(start, target) _patch_address(__FILE__, __LINE__, start, target);

#ifdef __cplusplus
}
#endif
