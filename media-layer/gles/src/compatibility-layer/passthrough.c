#include <GLES/gl.h>

#include "passthrough.h"

// Simple v1.1 -> v2.0 Passthrough Functions
GL_FUNC(glLineWidth, void, (GLfloat width));
void glLineWidth(GLfloat width) {
    real_glLineWidth()(width);
}
GL_FUNC(glBlendFunc, void, (GLenum sfactor, GLenum dfactor));
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    real_glBlendFunc()(sfactor, dfactor);
}
GL_FUNC(glClear, void, (GLbitfield mask));
void glClear(GLbitfield mask) {
    real_glClear()(mask);
}
GL_FUNC(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage));
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    real_glBufferData()(target, size, data, usage);
}
GL_FUNC(glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height));
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    real_glScissor()(x, y, width, height);
}
GL_FUNC(glTexParameteri, void, (GLenum target, GLenum pname, GLint param));
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    real_glTexParameteri()(target, pname, param);
}
GL_FUNC(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels));
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
    real_glTexImage2D()(target, level, internalformat, width, height, border, format, type, pixels);
}
GL_FUNC(glPolygonOffset, void, (GLfloat factor, GLfloat units));
void glPolygonOffset(GLfloat factor, GLfloat units) {
    real_glPolygonOffset()(factor, units);
}
GL_FUNC(glDepthRangef, void, (GLclampf near, GLclampf far));
void glDepthRangef(GLclampf near, GLclampf far) {
    real_glDepthRangef()(near, far);
}
GL_FUNC(glDepthFunc, void, (GLenum func));
void glDepthFunc(GLenum func) {
    real_glDepthFunc()(func);
}
GL_FUNC(glBindBuffer, void, (GLenum target, GLuint buffer));
void glBindBuffer(GLenum target, GLuint buffer) {
    real_glBindBuffer()(target, buffer);
}
GL_FUNC(glClearColor, void, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha));
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    real_glClearColor()(red, green, blue, alpha);
}
GL_FUNC(glDepthMask, void, (GLboolean flag));
void glDepthMask(GLboolean flag) {
    real_glDepthMask()(flag);
}
GL_FUNC(glHint, void, (GLenum target, GLenum mode));
void glHint(GLenum target, GLenum mode) {
    if (target != GL_PERSPECTIVE_CORRECTION_HINT) {
        real_glHint()(target, mode);
    }
}
GL_FUNC(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers));
void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
    real_glDeleteBuffers()(n, buffers);
}
GL_FUNC(glColorMask, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha));
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    real_glColorMask()(red, green, blue, alpha);
}
GL_FUNC(glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels));
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
    real_glTexSubImage2D()(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
GL_FUNC(glGenTextures, void, (GLsizei n, GLuint *textures));
void glGenTextures(GLsizei n, GLuint *textures) {
    real_glGenTextures()(n, textures);
}
GL_FUNC(glDeleteTextures, void, (GLsizei n, const GLuint *textures));
void glDeleteTextures(GLsizei n, const GLuint *textures) {
    real_glDeleteTextures()(n, textures);
}
GL_FUNC(glBindTexture, void, (GLenum target, GLuint texture));
void glBindTexture(GLenum target, GLuint texture) {
    real_glBindTexture()(target, texture);
}
GL_FUNC(glCullFace, void, (GLenum mode));
void glCullFace(GLenum mode) {
    real_glCullFace()(mode);
}
GL_FUNC(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height));
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    real_glViewport()(x, y, width, height);
}
GL_FUNC(glIsEnabled, GLboolean, (GLenum cap));
GLboolean glIsEnabled(GLenum cap) {
    return real_glIsEnabled()(cap);
}
GL_FUNC(glGetIntegerv, void, (GLenum pname, GLint *data));
void glGetIntegerv(GLenum pname, GLint *data) {
    real_glGetIntegerv()(pname, data);
}
GL_FUNC(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data));
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data) {
    real_glReadPixels()(x, y, width, height, format, type, data);
}
void glShadeModel(__attribute__((unused)) GLenum mode) {
    // Do Nothing
}
void glNormal3f(__attribute__((unused)) GLfloat nx, __attribute__((unused)) GLfloat ny, __attribute__((unused)) GLfloat nz) {
    // Do Nothing
}
