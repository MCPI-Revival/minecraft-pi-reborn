#include <libreborn/libreborn.h>

#include "state.h"
#include "passthrough.h"

// GL State
gl_state_t gl_state = {
    .color = {
        .red = 1,
        .green = 1,
        .blue = 1,
        .alpha = 1
    },
    .matrix_stacks = {
        .mode = GL_MODELVIEW
    },
    .alpha_test = 0,
    .texture_2d = 0,
    .fog = {
        .enabled = 0,
        .mode = GL_LINEAR,
        .color = {0, 0, 0, 0},
        .start = 0,
        .end = 1
    }
};

// Change Color
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    gl_state.color.red = red;
    gl_state.color.green = green;
    gl_state.color.blue = blue;
    gl_state.color.alpha = alpha;
}

// Array Pointer Storage
#define ARRAY_POINTER_FUNC(func, name) \
    void func(GLint size, GLenum type, GLsizei stride, const void *pointer) { \
        gl_state.array_pointers.name.size = size; \
        gl_state.array_pointers.name.type = type; \
        gl_state.array_pointers.name.stride = stride; \
        gl_state.array_pointers.name.pointer = pointer; \
    }
ARRAY_POINTER_FUNC(glVertexPointer, vertex)
ARRAY_POINTER_FUNC(glColorPointer, color)
ARRAY_POINTER_FUNC(glTexCoordPointer, tex_coord)
static array_pointer_t *get_array_pointer(GLenum array) {
    switch (array) {
        case GL_VERTEX_ARRAY: {
            return &gl_state.array_pointers.vertex;
        }
        case GL_COLOR_ARRAY: {
            return &gl_state.array_pointers.color;
        }
        case GL_TEXTURE_COORD_ARRAY: {
            return &gl_state.array_pointers.tex_coord;
        }
        default: {
            ERR("Unsupported Array Pointer: %i", array);
        }
    }
}
void glEnableClientState(GLenum array) {
    get_array_pointer(array)->enabled = 1;
}
void glDisableClientState(GLenum array) {
    get_array_pointer(array)->enabled = 0;
}

// Enable/Disable State
GL_FUNC(glEnable, void, (GLenum cap));
void glEnable(GLenum cap) {
    switch (cap) {
        case GL_ALPHA_TEST: {
            gl_state.alpha_test = 1;
            break;
        }
        case GL_TEXTURE_2D: {
            gl_state.texture_2d = 1;
            break;
        }
        case GL_COLOR_MATERIAL: {
            // Ignore
            break;
        }
        case GL_FOG: {
            gl_state.fog.enabled = 1;
            break;
        }
        default: {
            real_glEnable()(cap);
            break;
        }
    }
}
GL_FUNC(glDisable, void, (GLenum cap));
void glDisable(GLenum cap) {
    switch (cap) {
        case GL_ALPHA_TEST: {
            gl_state.alpha_test = 0;
            break;
        }
        case GL_TEXTURE_2D: {
            gl_state.texture_2d = 0;
            break;
        }
        case GL_COLOR_MATERIAL: {
            // Ignore
            break;
        }
        case GL_FOG: {
            gl_state.fog.enabled = 0;
            break;
        }
        default: {
            real_glDisable()(cap);
            break;
        }
    }
}
void glAlphaFunc(GLenum func, GLclampf ref) {
    if (func != GL_GREATER && ref != 0.1f) {
        ERR("Unsupported Alpha Function");
    }
}

// Fog
#define UNSUPPORTED_FOG() ERR("Unsupported Fog Configuration")
void glFogfv(GLenum pname, const GLfloat *params) {
    if (pname == GL_FOG_COLOR) {
        memcpy((void *) gl_state.fog.color, params, sizeof (gl_state.fog.color));
    } else {
        UNSUPPORTED_FOG();
    }
}
void glFogx(GLenum pname, GLfixed param) {
    if (pname == GL_FOG_MODE && (param == GL_LINEAR || param == GL_EXP)) {
        gl_state.fog.mode = param;
    } else {
        UNSUPPORTED_FOG();
    }
}
void glFogf(GLenum pname, GLfloat param) {
    switch (pname) {
        case GL_FOG_DENSITY:
        case GL_FOG_START: {
            gl_state.fog.start = param;
            break;
        }
        case GL_FOG_END: {
            gl_state.fog.end = param;
            break;
        }
        default: {
            UNSUPPORTED_FOG();
            break;
        }
    }
}

// Get Matrix Data
GL_FUNC(glGetFloatv, void, (GLenum pname, GLfloat *params));
void glGetFloatv(GLenum pname, GLfloat *params) {
    switch (pname) {
        case GL_MODELVIEW_MATRIX: {
            memcpy((void *) params, gl_state.matrix_stacks.model_view.stack[gl_state.matrix_stacks.model_view.i].data, MATRIX_DATA_SIZE);
            break;
        }
        case GL_PROJECTION_MATRIX: {
            memcpy((void *) params, gl_state.matrix_stacks.projection.stack[gl_state.matrix_stacks.projection.i].data, MATRIX_DATA_SIZE);
            break;
        }
        default: {
            real_glGetFloatv()(pname, params);
            break;
        }
    }
}
