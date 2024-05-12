#include <stdint.h>

#include <GLES/gl.h>
#include <libreborn/libreborn.h>

#include "common/common.h"

CALL(11, glFogfv, void, (GLenum pname, const GLfloat *params))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pname, (uint32_t) params);
#else
    GLenum pname = next_int();
    GLfloat *params = next_ptr();
    // Run
    func(pname, params);
#endif
}

// 'pointer' Is Only Supported As An Integer, Not As An Actual Pointer
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
#define CALL_GL_POINTER(unique_id, name) \
    CALL(unique_id, name, void, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
        trampoline(size, type, stride, (uint32_t) pointer); \
    }
#else
#define CALL_GL_POINTER(unique_id, name) \
    CALL(unique_id, name, unused, unused) \
        GLint size = next_int(); \
        GLenum type = next_int(); \
        GLsizei stride = next_int(); \
        const void *pointer = (const void *) (uint64_t) next_int(); \
        /* Run */ \
        func(size, type, stride, pointer); \
    }
#endif

CALL_GL_POINTER(12, glVertexPointer)

CALL(13, glLineWidth, void, (GLfloat width))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, width));
#else
    GLfloat width = next_float();
    // Run
    func(width);
#endif
}

CALL(14, glBlendFunc, void, (GLenum sfactor, GLenum dfactor))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(sfactor, dfactor);
#else
    GLenum sfactor = next_int();
    GLenum dfactor = next_int();
    // Run
    func(sfactor, dfactor);
#endif
}

CALL(15, glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(mode, first, count);
#else
    GLenum mode = next_int();
    GLint first = next_int();
    GLsizei count = next_int();
    // Run
    func(mode, first, count);
#endif
}

CALL(16, glColor4f, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, red), pun_to(uint32_t, green), pun_to(uint32_t, blue), pun_to(uint32_t, alpha));
#else
    GLfloat red = next_float();
    GLfloat green = next_float();
    GLfloat blue = next_float();
    GLfloat alpha = next_float();
    // Run
    func(red, green, blue, alpha);
#endif
}

CALL(17, glClear, void, (GLbitfield mask))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(mask);
#else
    GLbitfield mask = next_int();
    // Run
    func(mask);
#endif
}

CALL(18, glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(target, size, (uint32_t) data, usage);
#else
    GLenum target = next_int();
    GLsizeiptr size = next_int();
    const void *data = next_ptr();
    GLenum usage = next_int();
    // Run
    func(target, size, data, usage);
#endif
}

CALL(19, glFogx, void, (GLenum pname, GLfixed param))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pname, param);
#else
    GLenum pname = next_int();
    GLfixed param = next_int();
    // Run
    func(pname, param);
#endif
}

CALL(20, glFogf, void, (GLenum pname, GLfloat param))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pname, pun_to(uint32_t, param));
#else
    GLenum pname = next_int();
    GLfloat param = next_float();
    // Run
    func(pname, param);
#endif
}

CALL(21, glMatrixMode, void, (GLenum mode))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(mode);
#else
    GLenum mode = next_int();
    // Run
    func(mode);
#endif
}

CALL_GL_POINTER(22, glColorPointer)

CALL(23, glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(x, y, width, height);
#else
    GLint x = next_int();
    GLint y = next_int();
    GLsizei width = next_int();
    GLsizei height = next_int();
    // Run
    func(x, y, width, height);
#endif
}

CALL(24, glTexParameteri, void, (GLenum target, GLenum pname, GLint param))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(target, pname, param);
#else
    GLenum target = next_int();
    GLenum pname = next_int();
    GLint param = next_int();
    // Run
    func(target, pname, param);
#endif
}

CALL(25, glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(target, level, internalformat, width, height, border, format, type, (uint32_t) pixels);
#else
    GLenum target = next_int();
    GLint level = next_int();
    GLint internalformat = next_int();
    GLsizei width = next_int();
    GLsizei height = next_int();
    GLint border = next_int();
    GLenum format = next_int();
    GLenum type = next_int();
    const void *pixels = next_ptr();
    // Run
    func(target, level, internalformat, width, height, border, format, type, pixels);
#endif
}

CALL(26, glEnable, void, (GLenum cap))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(cap);
#else
    GLenum cap = next_int();
    // Run
    func(cap);
#endif
}

CALL(27, glEnableClientState, void, (GLenum array))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(array);
#else
    GLenum array = next_int();
    // Run
    func(array);
#endif
}

CALL(28, glPolygonOffset, void, (GLfloat factor, GLfloat units))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, factor), pun_to(uint32_t, units));
#else
    GLfloat factor = next_float();
    GLfloat units = next_float();
    // Run
    func(factor, units);
#endif
}

CALL_GL_POINTER(41, glTexCoordPointer)

CALL(29, glDisableClientState, void, (GLenum array))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(array);
#else
    GLenum array = next_int();
    // Run
    func(array);
#endif
}

CALL(30, glDepthRangef, void, (GLclampf near, GLclampf far))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, near), pun_to(uint32_t, far));
#else
    GLclampf near = next_float();
    GLclampf far = next_float();
    // Run
    func(near, far);
#endif
}

CALL(31, glDepthFunc, void, (GLenum func))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(func);
#else
    GLenum func2 = next_int();
    // Run
    func(func2);
#endif
}

CALL(32, glBindBuffer, void, (GLenum target, GLuint buffer))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(target, buffer);
#else
    GLenum target = next_int();
    GLenum buffer = next_int();
    // Run
    func(target, buffer);
#endif
}

CALL(33, glClearColor, void, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, red), pun_to(uint32_t, green), pun_to(uint32_t, blue), pun_to(uint32_t, alpha));
#else
    GLclampf red = next_float();
    GLclampf green = next_float();
    GLclampf blue = next_float();
    GLclampf alpha = next_float();
    // Run
    func(red, green, blue, alpha);
#endif
}

CALL(34, glPopMatrix, void, ())
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(35, glLoadIdentity, void, ())
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(36, glScalef, void, (GLfloat x, GLfloat y, GLfloat z))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, x), pun_to(uint32_t, y), pun_to(uint32_t, z));
#else
    GLfloat x = next_float();
    GLfloat y = next_float();
    GLfloat z = next_float();
    // Run
    func(x, y, z);
#endif
}

CALL(37, glPushMatrix, void, ())
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline();
#else
    // Run
    func();
#endif
}

CALL(38, glDepthMask, void, (GLboolean flag))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(flag);
#else
    GLboolean flag = next_int();
    // Run
    func(flag);
#endif
}

CALL(39, glHint, void, (GLenum target, GLenum mode))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(target, mode);
#else
    GLenum target = next_int();
    GLenum mode = next_int();
    // Run
    func(target, mode);
#endif
}

CALL(40, glMultMatrixf, void, (const GLfloat *m))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline((uint32_t) m);
#else
    GLfloat *m = next_ptr();
    // Run
    func(m);
#endif
}

CALL(42, glDeleteBuffers, void, (GLsizei n, const GLuint *buffers))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(n, (uint32_t) buffers);
#else
    GLsizei n = next_int();
    GLuint *buffers = next_ptr();
    // Run
    func(n, buffers);
#endif
}

CALL(43, glColorMask, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(red, green, blue, alpha);
#else
    GLboolean red = next_int();
    GLboolean green = next_int();
    GLboolean blue = next_int();
    GLboolean alpha = next_int();
    // Run
    func(red, green, blue, alpha);
#endif
}

CALL(44, glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(target, level, xoffset, yoffset, width, height, format, type, (uint32_t) pixels);
#else
    GLenum target = next_int();
    GLint level = next_int();
    GLint xoffset = next_int();
    GLint yoffset = next_int();
    GLsizei width = next_int();
    GLsizei height = next_int();
    GLenum format = next_int();
    GLenum type = next_int();
    const void *pixels = next_ptr();
    // Run
    func(target, level, xoffset, yoffset, width, height, format, type, pixels);
#endif
}

CALL(45, glGenTextures, void, (GLsizei n, GLuint *textures))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(n, (uint32_t) textures);
#else
    GLsizei n = next_int();
    GLuint *textures = next_ptr();
    // Run
    func(n, textures);
#endif
}

CALL(46, glDeleteTextures, void, (GLsizei n, const GLuint *textures))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(n, (uint32_t) textures);
#else
    GLsizei n = next_int();
    GLuint *textures = next_ptr();
    // Run
    func(n, textures);
#endif
}

CALL(47, glAlphaFunc, void, (GLenum func, GLclampf ref))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(func, pun_to(uint32_t, ref));
#else
    GLenum func2 = next_int();
    GLclampf ref = next_float();
    // Run
    func(func2, ref);
#endif
}

CALL(48, glGetFloatv, void, (GLenum pname, GLfloat *params))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pname, (uint32_t) params);
#else
    GLenum pname = next_int();
    GLfloat *params = next_ptr();
    // Run
    func(pname, params);
#endif
}

CALL(49, glBindTexture, void, (GLenum target, GLuint texture))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(target, texture);
#else
    GLenum target = next_int();
    GLuint texture = next_int();
    // Run
    func(target, texture);
#endif
}

CALL(50, glTranslatef, void, (GLfloat x, GLfloat y, GLfloat z))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, x), pun_to(uint32_t, y), pun_to(uint32_t, z));
#else
    GLfloat x = next_float();
    GLfloat y = next_float();
    GLfloat z = next_float();
    // Run
    func(x, y, z);
#endif
}

CALL(51, glShadeModel, void, (GLenum mode))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(mode);
#else
    GLenum mode = next_int();
    // Run
    func(mode);
#endif
}

CALL(52, glOrthof, void, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, left), pun_to(uint32_t, right), pun_to(uint32_t, bottom), pun_to(uint32_t, top), pun_to(uint32_t, near), pun_to(uint32_t, far));
#else
    GLfloat left = next_float();
    GLfloat right = next_float();
    GLfloat bottom = next_float();
    GLfloat top = next_float();
    GLfloat near = next_float();
    GLfloat far = next_float();
    // Run
    func(left, right, bottom, top, near, far);
#endif
}

CALL(53, glDisable, void, (GLenum cap))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(cap);
#else
    GLenum cap = next_int();
    // Run
    func(cap);
#endif
}

CALL(54, glCullFace, void, (GLenum mode))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(mode);
#else
    GLenum mode = next_int();
    // Run
    func(mode);
#endif
}

CALL(55, glRotatef, void, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, angle), pun_to(uint32_t, x), pun_to(uint32_t, y), pun_to(uint32_t, z));
#else
    GLfloat angle = next_float();
    GLfloat x = next_float();
    GLfloat y = next_float();
    GLfloat z = next_float();
    // Run
    func(angle, x, y, z);
#endif
}

CALL(56, glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(x, y, width, height);
#else
    GLint x = next_int();
    GLint y = next_int();
    GLsizei width = next_int();
    GLsizei height = next_int();
    // Run
    func(x, y, width, height);
#endif
}

CALL(57, glNormal3f, void, (GLfloat nx, GLfloat ny, GLfloat nz))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pun_to(uint32_t, nx), pun_to(uint32_t, ny), pun_to(uint32_t, nz));
#else
    GLfloat nx = next_float();
    GLfloat ny = next_float();
    GLfloat nz = next_float();
    // Run
    func(nx, ny, nz);
#endif
}

CALL(58, glIsEnabled, GLboolean, (GLenum cap))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    return trampoline(cap);
#else
    GLenum cap = next_int();
    // Run
    ret(func(cap));
#endif
}

CALL(61, glGetIntegerv, void, (GLenum pname, GLint *params))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(pname, (uint32_t) params);
#else
    GLenum pname = next_int();
    GLint *params = next_ptr();
    // Run
    func(pname, params);
#endif
}

CALL(65, glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(x, y, width, height, format, type, (uint32_t) data);
#else
    GLint x = next_int();
    GLint y = next_int();
    GLsizei width = next_int();
    GLsizei height = next_int();
    GLenum format = next_int();
    GLenum type = next_int();
    void *data = next_ptr();
    // Run
    func(x, y, width, height, format, type, data);
#endif
}

CALL(67, glGenBuffers, void, (GLsizei n, GLuint *buffers))
#if defined(MEDIA_LAYER_TRAMPOLINE_GUEST)
    trampoline(n, (uint32_t) buffers);
#else
    GLsizei n = next_int();
    GLuint *buffers = next_ptr();
    // Run
    func(n, buffers);
#endif
}
