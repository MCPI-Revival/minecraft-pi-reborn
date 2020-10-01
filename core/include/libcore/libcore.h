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

void *overwrite(void *start, void *target);
void revert_overwrite(void *start, void *original);
void patch(void *start, unsigned char patch[]);

#ifdef __cplusplus
}
#endif

#endif
