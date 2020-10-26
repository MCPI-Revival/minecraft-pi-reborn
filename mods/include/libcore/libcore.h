#ifndef LIBLOADER_H

#define LIBLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define HOOK(name, return_type, args) \
    typedef return_type (*name##_t)args; \
    static name##_t real_##name = NULL; \
    \
    __attribute__((__unused__)) static void ensure_##name() { \
        if (!real_##name) { \
            dlerror(); \
            real_##name = (name##_t) dlsym(RTLD_NEXT, #name); \
            if (!real_##name) { \
                fprintf(stderr, "Error Resolving Symbol: "#name": %s\n", dlerror()); \
                exit(1); \
            } \
        } \
    }; \
    \
    __attribute__((__used__)) return_type name args

#define INFO(msg, ...) fprintf(stderr, "[INFO]: " msg "\n", __VA_ARGS__);
#define ERR(msg, ...) fprintf(stderr, "[ERR]: " msg "\n", __VA_ARGS__); exit(1);

void *_overwrite(const char *file, int line, void *start, void *target);
#define overwrite(start, target) _overwrite(__FILE__, __LINE__, start, target);

void revert_overwrite(void *start, void *original);

void _patch(const char *file, int line, void *start, unsigned char patch[]);
#define patch(start, patch) _patch(__FILE__, __LINE__, start, patch);

void _patch_address(const char *file, int line, void *start, void *target);
#define patch_address(start, target) _patch_address(__FILE__, __LINE__, start, target);

#ifdef __cplusplus
}
#endif

#endif
