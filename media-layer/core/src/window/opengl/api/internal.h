#pragma once

#include <GLES/gl.h>
#include <libreborn/log.h>

#include "../../media.h"

// Load GL Function
#define GL_FUNC(name, return_type, args) \
    typedef return_type (*real_##name##_t)args; \
    static real_##name##_t real_##name() { \
        static real_##name##_t func = nullptr; \
        static unsigned int old_context = 0; \
        if (!func || old_context != media_context_id) { \
            old_context = media_context_id; \
            if (glfw_window == nullptr) { \
                IMPOSSIBLE(); \
            } \
            func = (real_##name##_t) glfwGetProcAddress(#name); \
            if (!func) { \
                ERR("Error Resolving OpenGL Function: " #name); \
            } \
        } \
        return func; \
    }
