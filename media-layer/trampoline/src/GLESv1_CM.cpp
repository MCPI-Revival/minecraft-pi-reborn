#include <cstdint>

#include <GLES/gl.h>
#include <libreborn/libreborn.h>

#include "common/common.h"

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static int get_glFogfv_params_length(const GLenum pname) {
    return pname == GL_FOG_COLOR ? 4 : 1;
}
#endif
CALL(11, media_glFogfv, void, (GLenum pname, const GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, copy_array(get_glFogfv_params_length(pname), params));
#else
    GLenum pname = args.next<GLenum>();
    const GLfloat *params = args.next_arr<GLfloat>();
    func(pname, params);
    return 0;
#endif
}

// Track GL State
struct gl_array_details_t {
    GLint size = -1;
    GLenum type = 0;
    GLsizei stride = 0;
    uint32_t pointer = 0;
};
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
struct gl_array_details_obj_t {
    gl_array_details_t media_glVertexPointer;
    gl_array_details_t media_glColorPointer;
    gl_array_details_t media_glTexCoordPointer;
    gl_array_details_t media_glNormalPointer;
};
static gl_array_details_obj_t gl_array_details;
#endif
struct gl_state_t {
    GLuint bound_array_buffer = 0;
    GLuint bound_texture = 0;
    bool vertex_array_enabled = false;
    bool color_array_enabled = false;
    bool tex_coord_array_enabled = false;
    bool normal_array_enabled = false;
    // Update State
    bool &get_array_enabled(const GLenum array) {
        switch (array) {
            case GL_VERTEX_ARRAY: {
                return vertex_array_enabled;
            }
            case GL_COLOR_ARRAY: {
                return color_array_enabled;
            }
            case GL_TEXTURE_COORD_ARRAY: {
                return tex_coord_array_enabled;
            }
            case GL_NORMAL_ARRAY: {
                return normal_array_enabled;
            }
            default: {
                ERR("Unsupported Array Type: %i", array);
            }
        }
    }
#ifndef MEDIA_LAYER_TRAMPOLINE_GUEST
    void send_array_to_driver(const GLenum array) {
        const bool state = get_array_enabled(array);
        if (state) {
            media_glEnableClientState(array);
        } else {
            media_glDisableClientState(array);
        }
    }
    void send_to_driver() {
        send_array_to_driver(GL_VERTEX_ARRAY);
        send_array_to_driver(GL_COLOR_ARRAY);
        send_array_to_driver(GL_TEXTURE_COORD_ARRAY);
        send_array_to_driver(GL_NORMAL_ARRAY);
        media_glBindBuffer(GL_ARRAY_BUFFER, bound_array_buffer);
        media_glBindTexture(GL_TEXTURE_2D, bound_texture);
    }
#endif
};
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static gl_state_t gl_state;
#endif

// Backup/Restore State (For Offscreen Rendering)
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static gl_state_t gl_state_backup;
static gl_array_details_obj_t gl_array_details_backup;
void _media_backup_gl_state() {
    gl_state_backup = gl_state;
    gl_array_details_backup = gl_array_details;
    gl_state = gl_state_t();
    gl_array_details = gl_array_details_obj_t();
}
void _media_restore_gl_state() {
    gl_state = gl_state_backup;
    gl_array_details = gl_array_details_backup;
}
#endif

// 'pointer' Is Only Supported As An Integer, Not As An Actual Pointer
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
#define CALL_GL_POINTER(unique_id, name) \
    CALL(unique_id, name, void, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
        gl_array_details_t &state = gl_array_details.name; \
        if (state.size != size || state.type != type || state.stride != stride || state.pointer != uint32_t(pointer)) { \
            state.size = size; \
            state.type = type; \
            state.stride = stride; \
            state.pointer = uint32_t(pointer); \
            trampoline(true, gl_state.bound_array_buffer, state); \
        } \
    }
#else
#define CALL_GL_POINTER(unique_id, name) \
    CALL(unique_id, name, unused, ()) \
        media_glBindBuffer(GL_ARRAY_BUFFER, args.next<GLuint>()); \
        gl_array_details_t state = args.next<gl_array_details_t>(); \
        func(state.size, state.type, state.stride, (const void *) uintptr_t(state.pointer)); \
        return 0; \
    }
#endif

CALL_GL_POINTER(12, media_glVertexPointer)

CALL(13, media_glLineWidth, void, (GLfloat width))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, width);
#else
    func(args.next<float>());
    return 0;
#endif
}

CALL(14, media_glBlendFunc, void, (GLenum sfactor, GLenum dfactor))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, sfactor, dfactor);
#else
    GLenum sfactor = args.next<GLenum>();
    GLenum dfactor = args.next<GLenum>();
    func(sfactor, dfactor);
    return 0;
#endif
}

CALL(15, media_glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state, mode, first, count);
#else
    gl_state_t gl_state = args.next<gl_state_t>();
    gl_state.send_to_driver();
    GLenum mode = args.next<GLenum>();
    GLint first = args.next<GLint>();
    GLsizei count = args.next<GLsizei>();
    func(mode, first, count);
    return 0;
#endif
}

CALL(70, media_glMultiDrawArrays, void, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state, mode, copy_array(drawcount, first), copy_array(drawcount, count));
#else
    gl_state_t gl_state = args.next<gl_state_t>();
    gl_state.send_to_driver();
    GLenum mode = args.next<GLenum>();
    uint32_t drawcount;
    const GLint *first = args.next_arr<GLint>(&drawcount);
    const GLsizei *count = args.next_arr<GLsizei>();
    func(mode, first, count, GLsizei(drawcount));
    return 0;
#endif
}

CALL(16, media_glColor4f, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, red, green, blue, alpha);
#else
    GLfloat red = args.next<float>();
    GLfloat green = args.next<float>();
    GLfloat blue = args.next<float>();
    GLfloat alpha = args.next<float>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(17, media_glClear, void, (GLbitfield mask))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mask);
#else
    func(args.next<GLbitfield>());
    return 0;
#endif
}

CALL(18, media_glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    static bool use_syscall = getenv(TRAMPOLINE_ARGUMENTS_PIPE_ENV) == nullptr;
    if (use_syscall) {
        trampoline(false, gl_state.bound_array_buffer, target, int32_t(size), uint32_t(data), usage);
    } else {
        trampoline(true, gl_state.bound_array_buffer, target, int32_t(size), copy_array(size, (unsigned char *) data), usage);
    }
#else
    media_glBindBuffer(GL_ARRAY_BUFFER, args.next<GLuint>());
    GLenum target = args.next<GLenum>();
    int32_t size = args.next<int32_t>();
#ifdef MCPI_RUNTIME_IS_QEMU
    const unsigned char *data = nullptr;
    uint32_t data_addr = args.next<uint32_t>();
    if (data_addr != 0) {
        data = (const unsigned char *) (uintptr_t) (QEMU_GUEST_BASE + data_addr);
    }
#else
    const unsigned char *data = args.next_arr<unsigned char>();
#endif
    GLenum usage = args.next<GLenum>();
    func(target, size, data, usage);
    return 0;
#endif
}

CALL(19, media_glFogx, void, (GLenum pname, GLfixed param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, param);
#else
    GLenum pname = args.next<GLenum>();
    GLfixed param = args.next<GLfixed>();
    func(pname, param);
    return 0;
#endif
}

CALL(20, media_glFogf, void, (GLenum pname, GLfloat param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, param);
#else
    GLenum pname = args.next<GLenum>();
    GLfloat param = args.next<GLfloat>();
    func(pname, param);
    return 0;
#endif
}

CALL(21, media_glMatrixMode, void, (GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL_GL_POINTER(22, media_glColorPointer)

CALL(23, media_glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, width, height);
#else
    GLint x = args.next<GLint>();
    GLint y = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    func(x, y, width, height);
    return 0;
#endif
}

CALL(24, media_glTexParameteri, void, (GLenum target, GLenum pname, GLint param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_texture, target, pname, param);
#else
    media_glBindTexture(GL_TEXTURE_2D, args.next<GLuint>());
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
        media_glGetIntegerv(is_upload ? GL_UNPACK_ALIGNMENT : GL_PACK_ALIGNMENT, &alignment);
        // Round
        line_size = ALIGN_UP(line_size, alignment);
    }
    // Return
    return line_size * height;
}

CALL(25, media_glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_texture, target, level, internalformat, width, height, border, format, type, copy_array(get_texture_size(width, height, format, type, true), (const unsigned char *) pixels));
#else
    media_glBindTexture(GL_TEXTURE_2D, args.next<GLuint>());
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

CALL(26, media_glEnable, void, (GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, cap);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
void media_glEnableClientState(const GLenum array) {
    gl_state.get_array_enabled(array) = true;
}
#endif

CALL(28, media_glPolygonOffset, void, (GLfloat factor, GLfloat units))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, factor, units);
#else
    GLfloat factor = args.next<GLfloat>();
    GLfloat units = args.next<GLfloat>();
    func(factor, units);
    return 0;
#endif
}

CALL_GL_POINTER(41, media_glTexCoordPointer)

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
void media_glDisableClientState(const GLenum array) {
    gl_state.get_array_enabled(array) = false;
    switch (array) {
        case GL_VERTEX_ARRAY: {
            gl_array_details.media_glVertexPointer.size = -1;
            break;
        }
        case GL_COLOR_ARRAY: {
            gl_array_details.media_glColorPointer.size = -1;
            break;
        }
        case GL_TEXTURE_COORD_ARRAY: {
            gl_array_details.media_glTexCoordPointer.size = -1;
            break;
        }
        case GL_NORMAL_ARRAY: {
            gl_array_details.media_glNormalPointer.size = -1;
            break;
        }
    }
}
#endif

CALL(30, media_glDepthRangef, void, (GLclampf near, GLclampf far))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, near, far);
#else
    GLclampf near = args.next<GLclampf>();
    GLclampf far = args.next<GLclampf>();
    func(near, far);
    return 0;
#endif
}

CALL(31, media_glDepthFunc, void, (GLenum func))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, func);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
void media_glBindBuffer(const GLenum target, const GLuint buffer) {
    if (target == GL_ARRAY_BUFFER) {
        gl_state.bound_array_buffer = buffer;
    } else {
        ERR("Unsupported Buffer Binding: %u", target);
    }
    gl_array_details.media_glVertexPointer.size = -1;
    gl_array_details.media_glColorPointer.size = -1;
    gl_array_details.media_glTexCoordPointer.size = -1;
    gl_array_details.media_glNormalPointer.size = -1;
}
#endif

CALL(33, media_glClearColor, void, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, red, green, blue, alpha);
#else
    GLclampf red = args.next<GLclampf>();
    GLclampf green = args.next<GLclampf>();
    GLclampf blue = args.next<GLclampf>();
    GLclampf alpha = args.next<GLclampf>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(34, media_glPopMatrix, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(35, media_glLoadIdentity, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(36, media_glScalef, void, (GLfloat x, GLfloat y, GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, z);
#else
    GLfloat x = args.next<GLfloat>();
    GLfloat y = args.next<GLfloat>();
    GLfloat z = args.next<GLfloat>();
    func(x, y, z);
    return 0;
#endif
}

CALL(37, media_glPushMatrix, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(38, media_glDepthMask, void, (GLboolean flag))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, flag);
#else
    func(args.next<GLboolean>());
    return 0;
#endif
}

CALL(39, media_glHint, void, (GLenum target, GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, target, mode);
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
CALL(40, media_glMultMatrixf, void, (const GLfloat *m))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(get_glMultMatrixf_size(), m));
#else
    func(args.next_arr<GLfloat>());
    return 0;
#endif
}

CALL(42, media_glDeleteBuffers, void, (GLsizei n, const GLuint *buffers))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(n, buffers));
#else
    uint32_t n;
    const GLuint *buffers = args.next_arr<GLuint>(&n);
    func(GLsizei(n), buffers);
    return 0;
#endif
}

CALL(43, media_glColorMask, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, red, green, blue, alpha);
#else
    GLboolean red = args.next<GLboolean>();
    GLboolean green = args.next<GLboolean>();
    GLboolean blue = args.next<GLboolean>();
    GLboolean alpha = args.next<GLboolean>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(44, media_glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_texture, target, level, xoffset, yoffset, width, height, format, type, copy_array(get_texture_size(width, height, format, type, true), (const unsigned char *) pixels));
#else
    media_glBindTexture(GL_TEXTURE_2D, args.next<GLuint>());
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

CALL(45, media_glGenTextures, void, (GLsizei n, GLuint *textures))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, n, uint32_t(textures));
#else
    GLsizei n = args.next<GLsizei>();
    GLuint *textures = new GLuint[n];
    func(n, textures);
    writer(args.next<uint32_t>(), textures, n * sizeof(GLuint));
    delete[] textures;
    return 0;
#endif
}

CALL(46, media_glDeleteTextures, void, (GLsizei n, const GLuint *textures))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(n, textures));
#else
    uint32_t n;
    const GLuint *textures = args.next_arr<GLuint>(&n);
    func(GLsizei(n), textures);
    return 0;
#endif
}

CALL(47, media_glAlphaFunc, void, (GLenum func, GLclampf ref))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, func, ref);
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
        case GL_ALIASED_LINE_WIDTH_RANGE: {
            return 2;
        }
        default: {
            ERR("Unsupported media_glGetFloatv Property: %u", pname);
        }
    }
}
#endif
CALL(48, media_glGetFloatv, void, (GLenum pname, GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, pname, uint32_t(params));
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

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
void media_glBindTexture(const GLenum target, const GLuint texture) {
    if (target == GL_TEXTURE_2D) {
        gl_state.bound_texture = texture;
    } else {
        ERR("Unsupported Texture Binding: %u", target);
    }
}
#endif

CALL(50, media_glTranslatef, void, (GLfloat x, GLfloat y, GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, z);
#else
    GLfloat x = args.next<GLfloat>();
    GLfloat y = args.next<GLfloat>();
    GLfloat z = args.next<GLfloat>();
    func(x, y, z);
    return 0;
#endif
}

CALL(51, media_glShadeModel, void, (GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(52, media_glOrthof, void, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, left, right, bottom, top, near, far);
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

CALL(53, media_glDisable, void, (GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, cap);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(54, media_glCullFace, void, (GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(55, media_glRotatef, void, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, angle, x, y, z);
#else
    GLfloat angle = args.next<GLfloat>();
    GLfloat x = args.next<GLfloat>();
    GLfloat y = args.next<GLfloat>();
    GLfloat z = args.next<GLfloat>();
    func(angle, x, y, z);
    return 0;
#endif
}

CALL(56, media_glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, width, height);
#else
    GLint x = args.next<GLint>();
    GLint y = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    func(x, y, width, height);
    return 0;
#endif
}

CALL(57, media_glNormal3f, void, (GLfloat nx, GLfloat ny, GLfloat nz))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, nx, ny, nz);
#else
    GLfloat nx = args.next<GLfloat>();
    GLfloat ny = args.next<GLfloat>();
    GLfloat nz = args.next<GLfloat>();
    func(nx, ny, nz);
    return 0;
#endif
}

CALL(58, media_glIsEnabled, GLboolean, (GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, cap);
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
            ERR("Unsupported media_glGetIntegerv Property: %u", pname);
        }
    }
}
#endif
CALL(61, media_glGetIntegerv, void, (GLenum pname, GLint *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    if (pname == GL_TEXTURE_BINDING_2D) {
        params[0] = gl_state.bound_texture;
        return;
    }
    trampoline(false, pname, uint32_t(params));
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

CALL(65, media_glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, x, y, width, height, format, type, uint32_t(data));
#else
    GLint x = args.next<GLint>();
    GLint y = args.next<GLint>();
    GLsizei width = args.next<GLsizei>();
    GLsizei height = args.next<GLsizei>();
    GLenum format = args.next<GLenum>();
    GLenum type = args.next<GLenum>();
    const int data_size = get_texture_size(width, height, format, type, false);
    unsigned char *data = new unsigned char[data_size];
    func(x, y, width, height, format, type, data);
    writer(args.next<uint32_t>(), data, data_size);
    delete[] data;
    return 0;
#endif
}

CALL(67, media_glGenBuffers, void, (GLsizei n, GLuint *buffers))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, n, uint32_t(buffers));
#else
    GLsizei n = args.next<GLsizei>();
    GLuint *buffers = new GLuint[n];
    func(n, buffers);
    writer(args.next<uint32_t>(), buffers, n * sizeof(GLuint));
    delete[] buffers;
    return 0;
#endif
}

CALL(69, media_glBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    static bool use_syscall = getenv(TRAMPOLINE_ARGUMENTS_PIPE_ENV) == nullptr;
    if (use_syscall) {
        trampoline(false, gl_state.bound_array_buffer, target, int32_t(offset), int32_t(size), uint32_t(data));
    } else {
        trampoline(true, gl_state.bound_array_buffer, target, int32_t(offset), copy_array(size, (unsigned char *) data));
    }
#else
    media_glBindBuffer(GL_ARRAY_BUFFER, args.next<GLuint>());
    GLenum target = args.next<GLenum>();
    int32_t offset = args.next<int32_t>();
#ifdef MCPI_RUNTIME_IS_QEMU
    int32_t size = args.next<int32_t>();
    uint32_t data_addr = args.next<uint32_t>();
    const unsigned char *data = (const unsigned char *) (uintptr_t) (QEMU_GUEST_BASE + data_addr);
#else
    uint32_t size;
    const unsigned char *data = args.next_arr<unsigned char>(&size);
#endif
    func(target, offset, size, data);
    return 0;
#endif
}

CALL(72, media_glNormalPointer, void, (GLenum type, GLsizei stride, const void *pointer))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    gl_array_details_t &state = gl_array_details.media_glNormalPointer;
    if (state.size == -1 || state.type != type || state.stride != stride || state.pointer != uint32_t(pointer)) {
        state.size = 0;
        state.type = type;
        state.stride = stride;
        state.pointer = uint32_t(pointer);
        trampoline(true, gl_state.bound_array_buffer, state);
    }
#else
    media_glBindBuffer(GL_ARRAY_BUFFER, args.next<GLuint>());
    gl_array_details_t state = args.next<gl_array_details_t>();
    func(state.type, state.stride, (const void *) uintptr_t(state.pointer));
    return 0;
#endif
}

CALL(73, media_glLightfv, void, (GLenum light, GLenum pname, const GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, light, pname, copy_array(4, params));
#else
    GLenum light = args.next<GLenum>();
    GLenum pname = args.next<GLenum>();
    const GLfloat *params = args.next_arr<GLfloat>();
    func(light, pname, params);
    return 0;
#endif
}

CALL(74, media_glColorMaterial, void, (GLenum face, GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, face, mode);
#else
    GLenum face = args.next<GLenum>();
    GLenum mode = args.next<GLenum>();
    func(face, mode);
    return 0;
#endif
}

CALL(75, media_glLightModelfv, void, (GLenum pname, const GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, copy_array(4, params));
#else
    GLenum pname = args.next<GLenum>();
    const GLfloat *params = args.next_arr<GLfloat>();
    func(pname, params);
    return 0;
#endif
}