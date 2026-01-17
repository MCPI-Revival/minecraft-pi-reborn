#pragma once

#include <GLES/gl.h>
#include <libreborn/log.h>

#include "../../media.h"

// Load GL Function
#define GL_FUNC(name, return_type, args) \
    typedef return_type (*real_##name##_t)args; \
    static real_##name##_t real_##name() { \
        static real_##name##_t func = nullptr; \
        if (!func) { \
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

// Some Functions Are Unsafe In Display Lists
MCPI_INTERNAL extern bool _media_in_display_list;
#define NOT_ALLOWED_IN_DISPLAY_LIST() \
    if (_media_in_display_list) [[unlikely]] ERR("Not Allowed In Display List: %s", __func__)
