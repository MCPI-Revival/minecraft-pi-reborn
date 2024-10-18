#include <GLES/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <libreborn/libreborn.h>

// Load GL Function
static char *gl_dlerror() {
    return (char *) "Unknown Error";
}
#define dlerror gl_dlerror
static void *gl_dlsysm(__attribute__((unused)) void *handle, __attribute__((unused)) const char *name) {
    return (void *) glfwGetProcAddress(name);
}
#define dlsym gl_dlsysm
#define GL_FUNC EXTERNAL_FUNC

// Passthrough Functions
GL_FUNC(glFogfv, void, (GLenum pname, const GLfloat *params))
void glFogfv(const GLenum pname, const GLfloat *params) {
    real_glFogfv()(pname, params);
}
GL_FUNC(glVertexPointer, void, (GLint size, GLenum type, GLsizei stride, const void *pointer))
void glVertexPointer(const GLint size, const GLenum type, const GLsizei stride, const void *pointer) {
    real_glVertexPointer()(size, type, stride, pointer);
}
GL_FUNC(glLineWidth, void, (GLfloat width))
void glLineWidth(const GLfloat width) {
    real_glLineWidth()(width);
}
GL_FUNC(glBlendFunc, void, (GLenum sfactor, GLenum dfactor))
void glBlendFunc(const GLenum sfactor, const GLenum dfactor) {
    real_glBlendFunc()(sfactor, dfactor);
}
GL_FUNC(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))
void glDrawArrays(const GLenum mode, const GLint first, const GLsizei count) {
    real_glDrawArrays()(mode, first, count);
}
GL_FUNC(glMultiDrawArraysEXT, void, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount))
void glMultiDrawArrays(const GLenum mode, const GLint *first, const GLsizei *count, const GLsizei drawcount) {
    real_glMultiDrawArraysEXT()(mode, first, count, drawcount);
}
GL_FUNC(glColor4f, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
void glColor4f(const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha) {
    real_glColor4f()(red, green, blue, alpha);
}
GL_FUNC(glClear, void, (GLbitfield mask))
void glClear(const GLbitfield mask) {
    real_glClear()(mask);
}
GL_FUNC(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
void glBufferData(const GLenum target, const GLsizeiptr size, const void *data, const GLenum usage) {
    real_glBufferData()(target, size, data, usage);
}
GL_FUNC(glBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data))
void glBufferSubData(const GLenum target, const GLintptr offset, const GLsizeiptr size, const void *data) {
    real_glBufferSubData()(target, offset, size, data);
}
GL_FUNC(glFogi, void, (GLenum pname, GLint param))
void glFogx(const GLenum pname, const GLfixed param) {
    real_glFogi()(pname, param);
}
GL_FUNC(glFogf, void, (GLenum pname, GLfloat param))
void glFogf(const GLenum pname, const GLfloat param) {
    real_glFogf()(pname, param);
}
GL_FUNC(glMatrixMode, void, (GLenum mode))
void glMatrixMode(const GLenum mode) {
    real_glMatrixMode()(mode);
}
GL_FUNC(glColorPointer, void, (GLint size, GLenum type, GLsizei stride, const void *pointer))
void glColorPointer(const GLint size, const GLenum type, const GLsizei stride, const void *pointer) {
    real_glColorPointer()(size, type, stride, pointer);
}
GL_FUNC(glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height))
void glScissor(const GLint x, const GLint y, const GLsizei width, const GLsizei height) {
    real_glScissor()(x, y, width, height);
}
GL_FUNC(glTexParameteri, void, (GLenum target, GLenum pname, GLint param))
void glTexParameteri(const GLenum target, const GLenum pname, const GLint param) {
    real_glTexParameteri()(target, pname, param);
}
GL_FUNC(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels))
void glTexImage2D(const GLenum target, const GLint level, const GLint internalformat, const GLsizei width, const GLsizei height, const GLint border, const GLenum format, const GLenum type, const void *pixels) {
    real_glTexImage2D()(target, level, internalformat, width, height, border, format, type, pixels);
}
GL_FUNC(glEnable, void, (GLenum cap))
void glEnable(const GLenum cap) {
    real_glEnable()(cap);
}
GL_FUNC(glEnableClientState, void, (GLenum array))
void glEnableClientState(const GLenum array) {
    real_glEnableClientState()(array);
}
GL_FUNC(glPolygonOffset, void, (GLfloat factor, GLfloat units))
void glPolygonOffset(const GLfloat factor, const GLfloat units) {
    real_glPolygonOffset()(factor, units);
}
GL_FUNC(glDisableClientState, void, (GLenum array))
void glDisableClientState(const GLenum array) {
    real_glDisableClientState()(array);
}
GL_FUNC(glDepthRange, void, (GLclampd near, GLclampd far))
void glDepthRangef(const GLclampf near, const GLclampf far) {
    real_glDepthRange()(near, far);
}
GL_FUNC(glDepthFunc, void, (GLenum func))
void glDepthFunc(const GLenum func) {
    real_glDepthFunc()(func);
}
GL_FUNC(glBindBuffer, void, (GLenum target, GLuint buffer))
void glBindBuffer(const GLenum target, const GLuint buffer) {
    real_glBindBuffer()(target, buffer);
}
GL_FUNC(glClearColor, void, (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha))
void glClearColor(const GLclampf red, const GLclampf green, const GLclampf blue, const GLclampf alpha) {
    real_glClearColor()(red, green, blue, alpha);
}
GL_FUNC(glPopMatrix, void, ())
void glPopMatrix() {
    real_glPopMatrix()();
}
GL_FUNC(glLoadIdentity, void, ())
void glLoadIdentity() {
    real_glLoadIdentity()();
}
GL_FUNC(glScalef, void, (GLfloat x, GLfloat y, GLfloat z))
void glScalef(const GLfloat x, const GLfloat y, const GLfloat z) {
    real_glScalef()(x, y, z);
}
GL_FUNC(glPushMatrix, void, ())
void glPushMatrix() {
    real_glPushMatrix()();
}
GL_FUNC(glDepthMask, void, (GLboolean flag))
void glDepthMask(const GLboolean flag) {
    real_glDepthMask()(flag);
}
GL_FUNC(glHint, void, (GLenum target, GLenum mode))
void glHint(const GLenum target, const GLenum mode) {
    real_glHint()(target, mode);
}
GL_FUNC(glMultMatrixf, void, (const GLfloat *m))
void glMultMatrixf(const GLfloat *m) {
    real_glMultMatrixf()(m);
}
GL_FUNC(glTexCoordPointer, void, (GLint size, GLenum type, GLsizei stride, const void *pointer))
void glTexCoordPointer(const GLint size, const GLenum type, const GLsizei stride, const void *pointer) {
    real_glTexCoordPointer()(size, type, stride, pointer);
}
GL_FUNC(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers))
void glDeleteBuffers(const GLsizei n, const GLuint *buffers) {
    real_glDeleteBuffers()(n, buffers);
}
GL_FUNC(glColorMask, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))
void glColorMask(const GLboolean red, const GLboolean green, const GLboolean blue, const GLboolean alpha) {
    real_glColorMask()(red, green, blue, alpha);
}
GL_FUNC(glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))
void glTexSubImage2D(const GLenum target, const GLint level, const GLint xoffset, const GLint yoffset, const GLsizei width, const GLsizei height, const GLenum format, const GLenum type, const void *pixels) {
    real_glTexSubImage2D()(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
GL_FUNC(glGenTextures, void, (GLsizei n, GLuint *textures))
void glGenTextures(const GLsizei n, GLuint *textures) {
    real_glGenTextures()(n, textures);
}
GL_FUNC(glDeleteTextures, void, (GLsizei n, const GLuint *textures))
void glDeleteTextures(const GLsizei n, const GLuint *textures) {
    real_glDeleteTextures()(n, textures);
}
GL_FUNC(glAlphaFunc, void, (GLenum func, GLclampf ref))
void glAlphaFunc(const GLenum func, const GLclampf ref) {
    real_glAlphaFunc()(func, ref);
}
GL_FUNC(glGetFloatv, void, (GLenum pname, GLfloat *params))
void glGetFloatv(const GLenum pname, GLfloat *params) {
    real_glGetFloatv()(pname, params);
}
GL_FUNC(glBindTexture, void, (GLenum target, GLuint texture))
void glBindTexture(const GLenum target, const GLuint texture) {
    real_glBindTexture()(target, texture);
}
GL_FUNC(glTranslatef, void, (GLfloat x, GLfloat y, GLfloat z))
void glTranslatef(const GLfloat x, const GLfloat y, const GLfloat z) {
    real_glTranslatef()(x, y, z);
}
GL_FUNC(glShadeModel, void, (GLenum mode))
void glShadeModel(const GLenum mode) {
    real_glShadeModel()(mode);
}
GL_FUNC(glOrtho, void, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far))
void glOrthof(const GLfloat left, const GLfloat right, const GLfloat bottom, const GLfloat top, const GLfloat near, const GLfloat far) {
    real_glOrtho()(left, right, bottom, top, near, far);
}
GL_FUNC(glDisable, void, (GLenum cap))
void glDisable(const GLenum cap) {
    real_glDisable()(cap);
}
GL_FUNC(glCullFace, void, (GLenum mode))
void glCullFace(const GLenum mode) {
    real_glCullFace()(mode);
}
GL_FUNC(glRotatef, void, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z))
void glRotatef(const GLfloat angle, const GLfloat x, const GLfloat y, const GLfloat z) {
    real_glRotatef()(angle, x, y, z);
}
GL_FUNC(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height))
void glViewport(const GLint x, const GLint y, const GLsizei width, const GLsizei height) {
    real_glViewport()(x, y, width, height);
}
GL_FUNC(glNormal3f, void, (GLfloat nx, GLfloat ny, GLfloat nz))
void glNormal3f(const GLfloat nx, const GLfloat ny, const GLfloat nz) {
    real_glNormal3f()(nx, ny, nz);
}
GL_FUNC(glIsEnabled, GLboolean, (GLenum cap))
GLboolean glIsEnabled(const GLenum cap) {
    return real_glIsEnabled()(cap);
}
GL_FUNC(glGetIntegerv, void, (GLenum pname, GLint *data))
void glGetIntegerv(const GLenum pname, GLint *data) {
    real_glGetIntegerv()(pname, data);
}
GL_FUNC(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data))
void glReadPixels(const GLint x, const GLint y, const GLsizei width, const GLsizei height, const GLenum format, const GLenum type, void *data) {
    real_glReadPixels()(x, y, width, height, format, type, data);
}
GL_FUNC(glGenBuffers, void, (GLsizei n, GLuint *buffers))
void glGenBuffers(const GLsizei n, GLuint *buffers) {
    real_glGenBuffers()(n, buffers);
}
GL_FUNC(glNormalPointer, void, (GLenum type, GLsizei stride, const void *pointer))
void glNormalPointer(const GLenum type, const GLsizei stride, const void *pointer) {
    real_glNormalPointer()(type, stride, pointer);
}