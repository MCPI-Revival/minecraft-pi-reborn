#include <cstdint>

#include <GLES/gl.h>
#include <libreborn/libreborn.h>

#include "common/common.h"

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static int get_glFogfv_params_length(GLenum pname) {
    return pname == GL_FOG_COLOR ? 4 : 1;
}
#endif
CALL(11, glFogfv, void, (GLenum pname, const GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(pname, copy_array(get_glFogfv_params_length(pname), params));
#else
    GLenum pname = args.next<GLenum>();
    const GLfloat *params = args.next_arr<GLfloat>();
    func(pname, params);
    return 0;
#endif
}

// 'pointer' Is Only Supported As An Integer, Not As An Actual Pointer
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
#define CALL_GL_POINTER(unique_id, name) \
    CALL(unique_id, name, void, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
        trampoline(size, type, stride, uint32_t(pointer)); \
    }
#else
#define CALL_GL_POINTER(unique_id, name) \
    CALL(unique_id, name, unused, unused) \
        GLint size = args.next<GLint>(); \
        GLenum type = args.next<GLenum>(); \
        GLsizei stride = args.next<GLsizei>(); \
        const void *pointer = (const void *) (uint64_t) args.next<uint32_t>(); \
        func(size, type, stride, pointer); \
        return 0; \
    }
#endif

CALL_GL_POINTER(12, glVertexPointer)

CALL(13, glLineWidth, void, (GLfloat width))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(width);
#else
    func(args.next<float>());
    return 0;
#endif
}

CALL(14, glBlendFunc, void, (GLenum sfactor, GLenum dfactor))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(sfactor, dfactor);
#else
    GLenum sfactor = args.next<GLenum>();
    GLenum dfactor = args.next<GLenum>();
    func(sfactor, dfactor);
    return 0;
#endif
}

CALL(15, glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(mode, first, count);
#else
    GLenum mode = args.next<GLenum>();
    GLint first = args.next<GLint>();
    GLsizei count = args.next<GLsizei>();
    func(mode, first, count);
    return 0;
#endif
}

CALL(16, glColor4f, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(red, green, blue, alpha);
#else
    GLfloat red = args.next<float>();
    GLfloat green = args.next<float>();
    GLfloat blue = args.next<float>();
    GLfloat alpha = args.next<float>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(17, glClear, void, (GLbitfield mask))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(mask);
#else
    func(args.next<GLbitfield>());
    return 0;
#endif
}

CALL(18, glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(target, copy_array(size, (unsigned char *) data), usage);
#else
    GLenum target = args.next<GLenum>();
    uint32_t size;
    const unsigned char *data = args.next_arr<unsigned char>(&size);
    GLenum usage = args.next<GLenum>();
    func(target, GLsizeiptr(size), data, usage);
    return 0;
#endif
}

CALL(19, glFogx, void, (GLenum pname, GLfixed param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(pname, param);
#else
    GLenum pname = args.next<GLenum>();
    GLfixed param = args.next<GLfixed>();
    func(pname, param);
    return 0;
#endif
}

CALL(20, glFogf, void, (GLenum pname, GLfloat param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(pname, param);
#else
    GLenum pname = args.next<GLenum>();
    GLfloat param = args.next<GLfloat>();
    func(pname, param);
    return 0;
#endif
}

CALL(21, glMatrixMode, void, (GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL_GL_POINTER(22, glColorPointer)

CALL(23, glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(x, y, width, height);
#else
    GLint x = args.next<GLint>();
    GLint y = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    func(x, y, width, height);
    return 0;
#endif
}

CALL(24, glTexParameteri, void, (GLenum target, GLenum pname, GLint param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(target, pname, param);
#else
    GLenum target = args.next<GLenum>();
    GLenum pname = args.next<GLenum>();
    GLint param = args.next<GLint>();
    func(target, pname, param);
    return 0;
#endif
}

// Get Size (In Memory) Of Specified Texture
static int get_texture_size(const GLsizei width, const GLsizei height, const GLenum format, const GLenum type, bool is_upload) {
    // Calculate Per-Pixel Multiplier
    int multiplier;
    if (type == GL_UNSIGNED_BYTE) {
        switch (format) {
            case GL_RGB: {
                multiplier = 3;
                break;
            }
            case GL_RGBA: {
                multiplier = 4;
                break;
            }
            default: {
                ERR("Unsupported Texture Format: %u", format);
            }
        }
    } else {
        // GL_UNSIGNED_SHORT_*
        multiplier = sizeof(unsigned short);
    }
    // Calculate Line Size
    int line_size = width * multiplier;
    {
        // Handle Alignment
        int alignment;
        glGetIntegerv(is_upload ? GL_UNPACK_ALIGNMENT : GL_PACK_ALIGNMENT, &alignment);
        // Round
        int diff = line_size % alignment;
        line_size = line_size + (alignment - diff);
    }
    // Return
    return line_size * height;
}

CALL(25, glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(target, level, internalformat, width, height, border, format, type, copy_array(get_texture_size(width, height, format, type, true), (const unsigned char *) pixels));
#else
    GLenum target = args.next<GLenum>();
    GLint level = args.next<GLint>();
    GLint internalformat = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    GLint border = args.next<GLint>();
    GLenum format = args.next<GLenum>();
    GLenum type = args.next<GLenum>();
    const unsigned char *pixels = args.next_arr<unsigned char>();
    func(target, level, internalformat, width, height, border, format, type, pixels);
    return 0;
#endif
}

CALL(26, glEnable, void, (GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(cap);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(27, glEnableClientState, void, (GLenum array))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(array);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(28, glPolygonOffset, void, (GLfloat factor, GLfloat units))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(factor, units);
#else
    GLfloat factor = args.next<GLfloat>();
    GLfloat units = args.next<GLfloat>();
    func(factor, units);
    return 0;
#endif
}

CALL_GL_POINTER(41, glTexCoordPointer)

CALL(29, glDisableClientState, void, (GLenum array))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(array);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(30, glDepthRangef, void, (GLclampf near, GLclampf far))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(near, far);
#else
    GLclampf near = args.next<GLclampf>();
    GLclampf far = args.next<GLclampf>();
    func(near, far);
    return 0;
#endif
}

CALL(31, glDepthFunc, void, (GLenum func))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(func);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(32, glBindBuffer, void, (GLenum target, GLuint buffer))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(target, buffer);
#else
    GLenum target = args.next<GLenum>();
    GLenum buffer = args.next<GLenum>();
    func(target, buffer);
    return 0;
#endif
}

CALL(33, glClearColor, void, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(red, green, blue, alpha);
#else
    GLclampf red = args.next<GLclampf>();
    GLclampf green = args.next<GLclampf>();
    GLclampf blue = args.next<GLclampf>();
    GLclampf alpha = args.next<GLclampf>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(34, glPopMatrix, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    func();
    return 0;
#endif
}

CALL(35, glLoadIdentity, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    func();
    return 0;
#endif
}

CALL(36, glScalef, void, (GLfloat x, GLfloat y, GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(x, y, z);
#else
    GLfloat x = args.next<GLfloat>();
    GLfloat y = args.next<GLfloat>();
    GLfloat z = args.next<GLfloat>();
    func(x, y, z);
    return 0;
#endif
}

CALL(37, glPushMatrix, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline();
#else
    func();
    return 0;
#endif
}

CALL(38, glDepthMask, void, (GLboolean flag))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(flag);
#else
    func(args.next<GLboolean>());
    return 0;
#endif
}

CALL(39, glHint, void, (GLenum target, GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(target, mode);
#else
    GLenum target = args.next<GLenum>();
    GLenum mode = args.next<GLenum>();
    func(target, mode);
    return 0;
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static int get_glMultMatrixf_size() {
    return 16;
}
#endif
CALL(40, glMultMatrixf, void, (const GLfloat *m))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(copy_array(get_glMultMatrixf_size(), m));
#else
    func(args.next_arr<GLfloat>());
    return 0;
#endif
}

CALL(42, glDeleteBuffers, void, (GLsizei n, const GLuint *buffers))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(copy_array(n, buffers));
#else
    uint32_t n;
    const GLuint *buffers = args.next_arr<GLuint>(&n);
    func(GLsizei(n), buffers);
    return 0;
#endif
}

CALL(43, glColorMask, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(red, green, blue, alpha);
#else
    GLboolean red = args.next<GLboolean>();
    GLboolean green = args.next<GLboolean>();
    GLboolean blue = args.next<GLboolean>();
    GLboolean alpha = args.next<GLboolean>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(44, glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(target, level, xoffset, yoffset, width, height, format, type, copy_array(get_texture_size(width, height, format, type, true), (const unsigned char *) pixels));
#else
    GLenum target = args.next<GLenum>();
    GLint level = args.next<GLint>();
    GLint xoffset = args.next<GLint>();
    GLint yoffset = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    GLenum format = args.next<GLenum>();
    GLenum type = args.next<GLenum>();
    const unsigned char *pixels = args.next_arr<unsigned char>();
    func(target, level, xoffset, yoffset, width, height, format, type, pixels);
    return 0;
#endif
}

CALL(45, glGenTextures, void, (GLsizei n, GLuint *textures))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(n, uint32_t(textures));
#else
    GLsizei n = args.next<GLsizei>();
    GLuint *textures = new GLuint[n];
    func(n, textures);
    writer(args.next<uint32_t>(), textures, n * sizeof(GLuint));
    delete[] textures;
    return 0;
#endif
}

CALL(46, glDeleteTextures, void, (GLsizei n, const GLuint *textures))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(copy_array(n, textures));
#else
    uint32_t n;
    const GLuint *textures = args.next_arr<GLuint>(&n);
    func(GLsizei(n), textures);
    return 0;
#endif
}

CALL(47, glAlphaFunc, void, (GLenum func, GLclampf ref))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(func, ref);
#else
    GLenum func2 = args.next<GLenum>();
    GLclampf ref = args.next<GLclampf>();
    func(func2, ref);
    return 0;
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_HOST
static int get_glGetFloatv_params_size(GLenum pname) {
    switch (pname) {
        case GL_MODELVIEW_MATRIX:
        case GL_PROJECTION_MATRIX: {
            return 16;
        }
        case GL_ALIASED_LINE_WIDTH_RANGE:
        case GL_SMOOTH_LINE_WIDTH_RANGE: {
            return 2;
        }
        default: {
            ERR("Unsupported glGetFloatv Property: %u", pname);
        }
    }
}
#endif
CALL(48, glGetFloatv, void, (GLenum pname, GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(pname, uint32_t(params));
#else
    GLenum pname = args.next<GLenum>();
    int size = get_glGetFloatv_params_size(pname);
    GLfloat *params = new GLfloat[size];
    func(pname, params);
    writer(args.next<uint32_t>(), params, size * sizeof(GLfloat));
    delete[] params;
    return 0;
#endif
}

CALL(49, glBindTexture, void, (GLenum target, GLuint texture))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(target, texture);
#else
    GLenum target = args.next<GLenum>();
    GLuint texture = args.next<GLuint>();
    func(target, texture);
    return 0;
#endif
}

CALL(50, glTranslatef, void, (GLfloat x, GLfloat y, GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(x, y, z);
#else
    GLfloat x = args.next<GLfloat>();
    GLfloat y = args.next<GLfloat>();
    GLfloat z = args.next<GLfloat>();
    func(x, y, z);
    return 0;
#endif
}

CALL(51, glShadeModel, void, (GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(52, glOrthof, void, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(left, right, bottom, top, near, far);
#else
    GLfloat left = args.next<GLfloat>();
    GLfloat right = args.next<GLfloat>();
    GLfloat bottom = args.next<GLfloat>();
    GLfloat top = args.next<GLfloat>();
    GLfloat near = args.next<GLfloat>();
    GLfloat far = args.next<GLfloat>();
    func(left, right, bottom, top, near, far);
    return 0;
#endif
}

CALL(53, glDisable, void, (GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(cap);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(54, glCullFace, void, (GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(55, glRotatef, void, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(angle, x, y, z);
#else
    GLfloat angle = args.next<GLfloat>();
    GLfloat x = args.next<GLfloat>();
    GLfloat y = args.next<GLfloat>();
    GLfloat z = args.next<GLfloat>();
    func(angle, x, y, z);
    return 0;
#endif
}

CALL(56, glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(x, y, width, height);
#else
    GLint x = args.next<GLint>();
    GLint y = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    func(x, y, width, height);
    return 0;
#endif
}

CALL(57, glNormal3f, void, (GLfloat nx, GLfloat ny, GLfloat nz))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(nx, ny, nz);
#else
    GLfloat nx = args.next<GLfloat>();
    GLfloat ny = args.next<GLfloat>();
    GLfloat nz = args.next<GLfloat>();
    func(nx, ny, nz);
    return 0;
#endif
}

CALL(58, glIsEnabled, GLboolean, (GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(cap);
#else
    return func(args.next<GLenum>());
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_HOST
static int get_glGetIntegerv_params_size(GLenum pname) {
    switch (pname) {
        case GL_TEXTURE_BINDING_2D:
        case GL_PACK_ALIGNMENT:
        case GL_UNPACK_ALIGNMENT: {
            return 1;
        }
        case GL_VIEWPORT: {
            return 4;
        }
        default: {
            ERR("Unsupported glGetIntegerv Property: %u", pname);
        }
    }
}
#endif
CALL(61, glGetIntegerv, void, (GLenum pname, GLint *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(pname, uint32_t(params));
#else
    GLenum pname = args.next<GLenum>();
    int size = get_glGetIntegerv_params_size(pname);
    GLint *params = new GLint[size];
    func(pname, params);
    writer(args.next<uint32_t>(), params, size * sizeof(GLint));
    delete[] params;
    return 0;
#endif
}

CALL(65, glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(x, y, width, height, format, type, uint32_t(data));
#else
    GLint x = args.next<GLint>();
    GLint y = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    GLenum format = args.next<GLenum>();
    GLenum type = args.next<GLenum>();
    int data_size = get_texture_size(width, height, format, type, false);
    unsigned char *data = new unsigned char[data_size];
    func(x, y, width, height, format, type, data);
    writer(args.next<uint32_t>(), data, data_size);
    delete[] data;
    return 0;
#endif
}

CALL(67, glGenBuffers, void, (GLsizei n, GLuint *buffers))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(n, uint32_t(buffers));
#else
    GLsizei n = args.next<GLsizei>();
    GLuint *buffers = new GLuint[n];
    func(n, buffers);
    writer(args.next<uint32_t>(), buffers, n * sizeof(GLuint));
    delete[] buffers;
    return 0;
#endif
}
