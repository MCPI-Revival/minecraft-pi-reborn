#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <libreborn/libreborn.h>

// Load GL Function
#define GL_FUNC(name, return_type, args) \
    typedef return_type (*real_##name##_t)args; \
    \
    __attribute__((__unused__)) static real_##name##_t real_##name() { \
        static real_##name##_t func = NULL; \
        if (!func) { \
            func = (real_##name##_t) glfwGetProcAddress(#name); \
            if (!func) { \
                ERR("Error Resolving GL Symbol: " #name ": %s", dlerror()); \
            } \
        } \
        return func; \
    }
