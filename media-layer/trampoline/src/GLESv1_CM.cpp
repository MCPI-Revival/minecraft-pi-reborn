#include <cstdint>

#include <GLES/gl.h>
#include <libreborn/log.h>
#include <libreborn/util/util.h>

#include "common/common.h"

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static int get_glFogfv_params_length(const GLenum pname) {
    return pname == GL_FOG_COLOR ? 4 : 1;
}
#endif
CALL(11, media_glFogfv, void, (const GLenum pname, const GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, copy_array(get_glFogfv_params_length(pname), params));
#else
    const GLenum pname = args.next<GLenum>();
    const GLfloat *params = args.next_arr<GLfloat>();
    func(pname, params);
    return 0;
#endif
}

// Track GL State
struct gl_array_details_t {
    bool dirty = false;
    bool enabled = false;
    GLint size = 0;
    GLenum type = 0;
    GLsizei stride = 0;
    uint32_t pointer = 0;
    void update(const bool enabled_, const GLint size_, const GLenum type_, const GLsizei stride_, const uint32_t pointer_) {
        if (enabled != enabled_ || size != size_ || type != type_ || stride != stride_ || pointer != pointer_) {
            // Array Details Have Changed!
            enabled = enabled_;
            size = size_;
            type = type_;
            stride = stride_;
            pointer = pointer_;
            dirty = true;
        }
    }
};
struct gl_state_t {
    GLuint bound_array_buffer = 0;
    GLuint bound_texture = 0;
    struct {
        gl_array_details_t media_glVertexPointer;
        gl_array_details_t media_glColorPointer;
        gl_array_details_t media_glTexCoordPointer;
        gl_array_details_t media_glNormalPointer;
        void set_all_dirty(const bool x) {
            media_glVertexPointer.dirty = x;
            media_glColorPointer.dirty = x;
            media_glTexCoordPointer.dirty = x;
            media_glNormalPointer.dirty = x;
        }
    } array_details;
    bool in_display_list = false;

    // Get Array Details
    gl_array_details_t &get_array_details(const GLenum array) {
        switch (array) {
            case GL_VERTEX_ARRAY: return array_details.media_glVertexPointer;
            case GL_COLOR_ARRAY: return array_details.media_glColorPointer;
            case GL_TEXTURE_COORD_ARRAY: return array_details.media_glTexCoordPointer;
            case GL_NORMAL_ARRAY: return array_details.media_glNormalPointer;
            default: ERR("Unsupported Array Type: %i", array);
        }
    }
    [[nodiscard]] const gl_array_details_t &get_array_details_const(const GLenum array) const {
        return const_cast<gl_state_t *>(this)->get_array_details(array);
    }

#ifndef MEDIA_LAYER_TRAMPOLINE_GUEST
    // Send Array State To Driver
    void send_array_to_driver(const GLenum array) const {
        const gl_array_details_t &state = get_array_details_const(array);
        if (!in_display_list && !state.dirty) {
            return;
        }
        if (state.enabled) {
            media_glEnableClientState(array);
            switch (array) {
                case GL_VERTEX_ARRAY: media_glVertexPointer(state.size, state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
                case GL_COLOR_ARRAY: media_glColorPointer(state.size, state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
                case GL_TEXTURE_COORD_ARRAY: media_glTexCoordPointer(state.size, state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
                case GL_NORMAL_ARRAY: media_glNormalPointer(state.type, state.stride, (void *) (uintptr_t) state.pointer); break;
                default: IMPOSSIBLE();
            }
        } else {
            media_glDisableClientState(array);
        }
    }
    // Send State To Driver
    void send_to_driver() const {
        send_array_to_driver(GL_VERTEX_ARRAY);
        send_array_to_driver(GL_COLOR_ARRAY);
        send_array_to_driver(GL_TEXTURE_COORD_ARRAY);
        send_array_to_driver(GL_NORMAL_ARRAY);
        if (!in_display_list) {
            media_glBindBuffer(GL_ARRAY_BUFFER, bound_array_buffer);
            media_glBindTexture(GL_TEXTURE_2D, bound_texture);
        }
    }
#endif
};
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static gl_state_t gl_state;
#endif

// Backup/Restore State (For Offscreen Rendering)
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
static gl_state_t gl_state_backup;
void _media_backup_gl_state() {
    gl_state_backup = gl_state;
    gl_state = gl_state_t();
}
void _media_restore_gl_state() {
    gl_state = gl_state_backup;
}
#endif

// 'pointer' Is Only Supported As An Integer, Not As An Actual Pointer
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
#define CALL_GL_POINTER(name) \
    void name(GLint size, GLenum type, GLsizei stride, const void *pointer) { \
        gl_array_details_t &obj = gl_state.array_details.name; \
        obj.update(obj.enabled, size, type, stride, uint32_t(pointer)); \
    }
#else
#define CALL_GL_POINTER(name)
#endif

CALL_GL_POINTER(media_glVertexPointer)

CALL(13, media_glLineWidth, void, (const GLfloat width))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, width);
#else
    func(args.next<float>());
    return 0;
#endif
}

CALL(14, media_glBlendFunc, void, (const GLenum sfactor, const GLenum dfactor))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, sfactor, dfactor);
#else
    const GLenum sfactor = args.next<GLenum>();
    const GLenum dfactor = args.next<GLenum>();
    func(sfactor, dfactor);
    return 0;
#endif
}

CALL(15, media_glDrawArrays, void, (const GLenum mode, const GLint first, const GLsizei count))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state, mode, first, count);
    gl_state.array_details.set_all_dirty(false);
#else
    const gl_state_t &gl_state = args.next<gl_state_t>();
    gl_state.send_to_driver();
    const GLenum mode = args.next<GLenum>();
    const GLint first = args.next<GLint>();
    const GLsizei count = args.next<GLsizei>();
    func(mode, first, count);
    return 0;
#endif
}

CALL(16, media_glColor4f, void, (const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, red, green, blue, alpha);
#else
    const GLfloat red = args.next<float>();
    const GLfloat green = args.next<float>();
    const GLfloat blue = args.next<float>();
    const GLfloat alpha = args.next<float>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(17, media_glClear, void, (const GLbitfield mask))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mask);
#else
    func(args.next<GLbitfield>());
    return 0;
#endif
}

CALL(18, media_glBufferData, void, (const GLenum target, const GLsizeiptr size, const void *data, const GLenum usage))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_array_buffer, target, int32_t(size), copy_array(size, (unsigned char *) data), usage);
#else
    media_glBindBuffer(GL_ARRAY_BUFFER, args.next<GLuint>());
    const GLenum target = args.next<GLenum>();
    const int32_t size = args.next<int32_t>();
    const unsigned char *data = args.next_arr<unsigned char>();
    const GLenum usage = args.next<GLenum>();
    func(target, size, data, usage);
    return 0;
#endif
}

CALL(19, media_glFogx, void, (const GLenum pname, const GLfixed param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, param);
#else
    const GLenum pname = args.next<GLenum>();
    const GLfixed param = args.next<GLfixed>();
    func(pname, param);
    return 0;
#endif
}

CALL(20, media_glFogf, void, (const GLenum pname, const GLfloat param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, param);
#else
    const GLenum pname = args.next<GLenum>();
    const GLfloat param = args.next<GLfloat>();
    func(pname, param);
    return 0;
#endif
}

CALL(21, media_glMatrixMode, void, (const GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL_GL_POINTER(media_glColorPointer)

CALL(23, media_glScissor, void, (const GLint x, const GLint y, const GLsizei width, const GLsizei height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, width, height);
#else
    const GLint x = args.next<GLint>();
    const GLint y = args.next<GLint>();
    const GLsizei width = args.next<GLsizei>();
    const GLsizei height = args.next<GLsizei>();
    func(x, y, width, height);
    return 0;
#endif
}

CALL(24, media_glTexParameteri, void, (const GLenum target, const GLenum pname, const GLint param))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_texture, target, pname, param);
#else
    media_glBindTexture(GL_TEXTURE_2D, args.next<GLuint>());
    const GLenum target = args.next<GLenum>();
    const GLenum pname = args.next<GLenum>();
    const GLint param = args.next<GLint>();
    func(target, pname, param);
    return 0;
#endif
}

// Get Size (In Memory) Of Specified Texture
template <bool is_upload>
static int get_texture_size(const GLsizei width, const GLsizei height, const GLenum format, const GLenum type) {
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
    static int alignment;
    static bool loaded = false;
    if (!loaded) {
        media_glGetIntegerv(is_upload ? GL_UNPACK_ALIGNMENT : GL_PACK_ALIGNMENT, &alignment);
        loaded = true;
    }
    line_size = align_up(line_size, alignment);
    // Return
    return line_size * height;
}

CALL(25, media_glTexImage2D, void, (const GLenum target, const GLint level, const GLint internalformat, const GLsizei width, const GLsizei height, const GLint border, const GLenum format, const GLenum type, const void *pixels))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_texture, target, level, internalformat, width, height, border, format, type, copy_array(get_texture_size<true>(width, height, format, type), (const unsigned char *) pixels));
#else
    media_glBindTexture(GL_TEXTURE_2D, args.next<GLuint>());
    const GLenum target = args.next<GLenum>();
    const GLint level = args.next<GLint>();
    const GLint internalformat = args.next<GLint>();
    const GLsizei width = args.next<GLsizei>();
    const GLsizei height = args.next<GLsizei>();
    const GLint border = args.next<GLint>();
    const GLenum format = args.next<GLenum>();
    const GLenum type = args.next<GLenum>();
    const unsigned char *pixels = args.next_arr<unsigned char>();
    func(target, level, internalformat, width, height, border, format, type, pixels);
    return 0;
#endif
}

CALL(26, media_glEnable, void, (const GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, cap);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
void media_glEnableClientState(const GLenum array) {
    gl_array_details_t &details = gl_state.get_array_details(array);
    details.update(true, details.size, details.type, details.stride, details.pointer);
}
#endif

CALL(28, media_glPolygonOffset, void, (const GLfloat factor, const GLfloat units))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, factor, units);
#else
    const GLfloat factor = args.next<GLfloat>();
    const GLfloat units = args.next<GLfloat>();
    func(factor, units);
    return 0;
#endif
}

CALL_GL_POINTER(media_glTexCoordPointer)

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
void media_glDisableClientState(const GLenum array) {
    gl_array_details_t &details = gl_state.get_array_details(array);
    details.update(false, details.size, details.type, details.stride, details.pointer);
}
#endif

CALL(30, media_glDepthRangef, void, (const GLclampf near, const GLclampf far))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, near, far);
#else
    const GLclampf near = args.next<GLclampf>();
    const GLclampf far = args.next<GLclampf>();
    func(near, far);
    return 0;
#endif
}

CALL(31, media_glDepthFunc, void, (const GLenum func))
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
        GLuint &bound_buffer = gl_state.bound_array_buffer;
        if (buffer != bound_buffer) {
            bound_buffer = buffer;
            gl_state.array_details.set_all_dirty(true);
        }
    } else {
        ERR("Unsupported Buffer Binding: %u", target);
    }
}
#endif

CALL(33, media_glClearColor, void, (const GLclampf red, const GLclampf green, const GLclampf blue, const GLclampf alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, red, green, blue, alpha);
#else
    const GLclampf red = args.next<GLclampf>();
    const GLclampf green = args.next<GLclampf>();
    const GLclampf blue = args.next<GLclampf>();
    const GLclampf alpha = args.next<GLclampf>();
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

CALL(36, media_glScalef, void, (const GLfloat x, const GLfloat y, const GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, z);
#else
    const GLfloat x = args.next<GLfloat>();
    const GLfloat y = args.next<GLfloat>();
    const GLfloat z = args.next<GLfloat>();
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

CALL(38, media_glDepthMask, void, (const GLboolean flag))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, flag);
#else
    func(args.next<GLboolean>());
    return 0;
#endif
}

CALL(39, media_glHint, void, (const GLenum target, const GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, target, mode);
#else
    const GLenum target = args.next<GLenum>();
    const GLenum mode = args.next<GLenum>();
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

CALL(42, media_glDeleteBuffers, void, (const GLsizei n, const GLuint *buffers))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(n, buffers));
#else
    uint32_t n;
    const GLuint *buffers = args.next_arr<GLuint>(&n);
    func(GLsizei(n), buffers);
    return 0;
#endif
}

CALL(43, media_glColorMask, void, (const GLboolean red, const GLboolean green, const GLboolean blue, const GLboolean alpha))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, red, green, blue, alpha);
#else
    const GLboolean red = args.next<GLboolean>();
    const GLboolean green = args.next<GLboolean>();
    const GLboolean blue = args.next<GLboolean>();
    const GLboolean alpha = args.next<GLboolean>();
    func(red, green, blue, alpha);
    return 0;
#endif
}

CALL(44, media_glTexSubImage2D, void, (const GLenum target, const GLint level, const GLint xoffset, const GLint yoffset, const GLsizei width, const GLsizei height, const GLenum format, const GLenum type, const void *pixels))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_texture, target, level, xoffset, yoffset, width, height, format, type, copy_array(get_texture_size<true>(width, height, format, type), (const unsigned char *) pixels));
#else
    media_glBindTexture(GL_TEXTURE_2D, args.next<GLuint>());
    const GLenum target = args.next<GLenum>();
    const GLint level = args.next<GLint>();
    const GLint xoffset = args.next<GLint>();
    const GLint yoffset = args.next<GLint>();
    const GLsizei width = args.next<GLsizei>();
    const GLsizei height = args.next<GLsizei>();
    const GLenum format = args.next<GLenum>();
    const GLenum type = args.next<GLenum>();
    const unsigned char *pixels = args.next_arr<unsigned char>();
    func(target, level, xoffset, yoffset, width, height, format, type, pixels);
    return 0;
#endif
}

CALL(45, media_glGenTextures, void, (const GLsizei n, GLuint *textures))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, n, uint32_t(textures));
#else
    const GLsizei n = args.next<GLsizei>();
    GLuint *textures = new GLuint[n];
    func(n, textures);
    writer(args.next<uint32_t>(), textures, n * sizeof(GLuint));
    delete[] textures;
    return 0;
#endif
}

CALL(46, media_glDeleteTextures, void, (const GLsizei n, const GLuint *textures))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, copy_array(n, textures));
#else
    uint32_t n;
    const GLuint *textures = args.next_arr<GLuint>(&n);
    func(GLsizei(n), textures);
    return 0;
#endif
}

CALL(47, media_glAlphaFunc, void, (GLenum func, const GLclampf ref))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, func, ref);
#else
    const GLenum func2 = args.next<GLenum>();
    const GLclampf ref = args.next<GLclampf>();
    func(func2, ref);
    return 0;
#endif
}

CALL(48, media_glGetFloatv, void, (const GLenum pname, GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, pname, uint32_t(params));
#else
    const GLenum pname = args.next<GLenum>();
    constexpr int matrx_size = 16;
    GLfloat params[matrx_size];
    func(pname, params);
    const int size = pname == GL_ALIASED_LINE_WIDTH_RANGE ? 2 : matrx_size;
    writer(args.next<uint32_t>(), params, size * sizeof(GLfloat));
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

CALL(50, media_glTranslatef, void, (const GLfloat x, const GLfloat y, const GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, z);
#else
    const GLfloat x = args.next<GLfloat>();
    const GLfloat y = args.next<GLfloat>();
    const GLfloat z = args.next<GLfloat>();
    func(x, y, z);
    return 0;
#endif
}

CALL(51, media_glShadeModel, void, (const GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(52, media_glOrthof, void, (const GLfloat left, const GLfloat right, const GLfloat bottom, const GLfloat top, const GLfloat near, const GLfloat far))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, left, right, bottom, top, near, far);
#else
    const GLfloat left = args.next<GLfloat>();
    const GLfloat right = args.next<GLfloat>();
    const GLfloat bottom = args.next<GLfloat>();
    const GLfloat top = args.next<GLfloat>();
    const GLfloat near = args.next<GLfloat>();
    const GLfloat far = args.next<GLfloat>();
    func(left, right, bottom, top, near, far);
    return 0;
#endif
}

CALL(53, media_glDisable, void, (const GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, cap);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(54, media_glCullFace, void, (const GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, mode);
#else
    func(args.next<GLenum>());
    return 0;
#endif
}

CALL(55, media_glRotatef, void, (const GLfloat angle, const GLfloat x, const GLfloat y, const GLfloat z))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, angle, x, y, z);
#else
    const GLfloat angle = args.next<GLfloat>();
    const GLfloat x = args.next<GLfloat>();
    const GLfloat y = args.next<GLfloat>();
    const GLfloat z = args.next<GLfloat>();
    func(angle, x, y, z);
    return 0;
#endif
}

CALL(56, media_glViewport, void, (const GLint x, const GLint y, const GLsizei width, const GLsizei height))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, x, y, width, height);
#else
    const GLint x = args.next<GLint>();
    const GLint y = args.next<GLint>();
    const GLsizei width = args.next<GLsizei>();
    const GLsizei height = args.next<GLsizei>();
    func(x, y, width, height);
    return 0;
#endif
}

CALL(57, media_glNormal3f, void, (const GLfloat nx, const GLfloat ny, const GLfloat nz))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, nx, ny, nz);
#else
    const GLfloat nx = args.next<GLfloat>();
    const GLfloat ny = args.next<GLfloat>();
    const GLfloat nz = args.next<GLfloat>();
    func(nx, ny, nz);
    return 0;
#endif
}

CALL(58, media_glIsEnabled, GLboolean, (const GLenum cap))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, cap);
#else
    return func(args.next<GLenum>());
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_HOST
static int get_glGetIntegerv_params_size(const GLenum pname) {
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
CALL(61, media_glGetIntegerv, void, (const GLenum pname, GLint *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    if (pname == GL_TEXTURE_BINDING_2D) {
        params[0] = GLint(gl_state.bound_texture);
        return;
    }
    trampoline(false, pname, uint32_t(params));
#else
    const GLenum pname = args.next<GLenum>();
    const int size = get_glGetIntegerv_params_size(pname);
    GLint *params = new GLint[size];
    func(pname, params);
    writer(args.next<uint32_t>(), params, size * sizeof(GLint));
    delete[] params;
    return 0;
#endif
}

CALL(65, media_glReadPixels, void, (const GLint x, const GLint y, const GLsizei width, const GLsizei height, const GLenum format, const GLenum type, void *data))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, x, y, width, height, format, type, uint32_t(data));
#else
    const GLint x = args.next<GLint>();
    const GLint y = args.next<GLint>();
    const GLsizei width = args.next<GLsizei>();
    const GLsizei height = args.next<GLsizei>();
    const GLenum format = args.next<GLenum>();
    const GLenum type = args.next<GLenum>();
    const int data_size = get_texture_size<false>(width, height, format, type);
    unsigned char *data = new unsigned char[data_size];
    func(x, y, width, height, format, type, data);
    writer(args.next<uint32_t>(), data, data_size);
    delete[] data;
    return 0;
#endif
}

CALL(67, media_glGenBuffers, void, (const GLsizei n, GLuint *buffers))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(false, n, uint32_t(buffers));
#else
    const GLsizei n = args.next<GLsizei>();
    GLuint *buffers = new GLuint[n];
    func(n, buffers);
    writer(args.next<uint32_t>(), buffers, n * sizeof(GLuint));
    delete[] buffers;
    return 0;
#endif
}

CALL(69, media_glBufferSubData, void, (const GLenum target, const GLintptr offset, const GLsizeiptr size, const void *data))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, gl_state.bound_array_buffer, target, int32_t(offset), copy_array(size, (unsigned char *) data));
#else
    media_glBindBuffer(GL_ARRAY_BUFFER, args.next<GLuint>());
    const GLenum target = args.next<GLenum>();
    const int32_t offset = args.next<int32_t>();
    uint32_t size;
    const unsigned char *data = args.next_arr<unsigned char>(&size);
    func(target, offset, size, data);
    return 0;
#endif
}

#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
void media_glNormalPointer(const GLenum type, const GLsizei stride, const void *pointer) {
    gl_array_details_t &obj = gl_state.array_details.media_glNormalPointer;
    obj.update(obj.enabled, 0, type, stride, uint32_t(pointer));
}
#endif

CALL(73, media_glLightfv, void, (const GLenum light, const GLenum pname, const GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, light, pname, copy_array(4, params));
#else
    const GLenum light = args.next<GLenum>();
    const GLenum pname = args.next<GLenum>();
    const GLfloat *params = args.next_arr<GLfloat>();
    func(light, pname, params);
    return 0;
#endif
}

CALL(74, media_glColorMaterial, void, (const GLenum face, const GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, face, mode);
#else
    const GLenum face = args.next<GLenum>();
    const GLenum mode = args.next<GLenum>();
    func(face, mode);
    return 0;
#endif
}

CALL(75, media_glLightModelfv, void, (const GLenum pname, const GLfloat *params))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, pname, copy_array(4, params));
#else
    const GLenum pname = args.next<GLenum>();
    const GLfloat *params = args.next_arr<GLfloat>();
    func(pname, params);
    return 0;
#endif
}

CALL(81, media_glGenLists, GLuint, (const GLsizei range))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    return trampoline(false, range);
#else
    return func(args.next<GLsizei>());
#endif
}

CALL(82, media_glDeleteLists, void, (const GLuint list, const GLsizei range))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    trampoline(true, list, range);
#else
    const GLuint list = args.next<GLuint>();
    const GLsizei range = args.next<GLsizei>();
    func(list, range);
    return 0;
#endif
}

CALL(83, media_glNewList, void, (const GLuint list, const GLenum mode))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    gl_state.in_display_list = true;
    trampoline(true, list, mode);
#else
    const GLuint list = args.next<GLuint>();
    const GLenum mode = args.next<GLenum>();
    func(list, mode);
    return 0;
#endif
}

CALL(84, media_glEndList, void, ())
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    gl_state.in_display_list = false;
    trampoline(true);
#else
    func();
    return 0;
#endif
}

CALL(85, media_glCallLists, void, (const GLsizei n, const GLenum type, const GLvoid *lists))
#ifdef MEDIA_LAYER_TRAMPOLINE_GUEST
    if (type != GL_UNSIGNED_INT) {
        IMPOSSIBLE();
    }
    const GLuint *int_lists = (const GLuint *) lists;
    trampoline(true, gl_state, type, copy_array(n, int_lists));
    gl_state.array_details.set_all_dirty(false);
#else
    const gl_state_t &gl_state = args.next<gl_state_t>();
    gl_state.send_to_driver();
    const GLenum type = args.next<GLenum>();
    uint32_t n;
    const GLuint *lists = args.next_arr<GLuint>(&n);
    func(GLsizei(n), type, lists);
    return 0;
#endif
}