#include <GLES/gl.h>

#include "../dependencies/gles-compatibility-layer/src/passthrough.h"

GL_FUNC(glFogfv, void, (GLenum pname, const GLfloat *params));
void glFogfv(GLenum pname, const GLfloat *params) {
    real_glFogfv()(pname, params);
}
GL_FUNC(glVertexPointer, void, (GLint size, GLenum type, GLsizei stride, const void *pointer));
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    real_glVertexPointer()(size, type, stride, pointer);
}
GL_FUNC(glLineWidth, void, (GLfloat width));
void glLineWidth(GLfloat width) {
    real_glLineWidth()(width);
}
GL_FUNC(glBlendFunc, void, (GLenum sfactor, GLenum dfactor));
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    real_glBlendFunc()(sfactor, dfactor);
}
GL_FUNC(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count));
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    real_glDrawArrays()(mode, first, count);
}
GL_FUNC(glColor4f, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha));
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    real_glColor4f()(red, green, blue, alpha);
}
GL_FUNC(glClear, void, (GLbitfield mask));
void glClear(GLbitfield mask) {
    real_glClear()(mask);
}
GL_FUNC(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage));
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    real_glBufferData()(target, size, data, usage);
}
GL_FUNC(glFogx, void, (GLenum pname, GLfixed param));
void glFogx(GLenum pname, GLfixed param) {
    real_glFogx()(pname, param);
}
GL_FUNC(glFogf, void, (GLenum pname, GLfloat param));
void glFogf(GLenum pname, GLfloat param) {
    real_glFogf()(pname, param);
}
GL_FUNC(glMatrixMode, void, (GLenum mode));
void glMatrixMode(GLenum mode) {
    real_glMatrixMode()(mode);
}
GL_FUNC(glColorPointer, void, (GLint size, GLenum type, GLsizei stride, const void *pointer));
void glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    real_glColorPointer()(size, type, stride, pointer);
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
GL_FUNC(glEnable, void, (GLenum cap));
void glEnable(GLenum cap) {
    real_glEnable()(cap);
}
GL_FUNC(glEnableClientState, void, (GLenum array));
void glEnableClientState(GLenum array) {
    real_glEnableClientState()(array);
}
GL_FUNC(glPolygonOffset, void, (GLfloat factor, GLfloat units));
void glPolygonOffset(GLfloat factor, GLfloat units) {
    real_glPolygonOffset()(factor, units);
}
GL_FUNC(glDisableClientState, void, (GLenum array));
void glDisableClientState(GLenum array) {
    real_glDisableClientState()(array);
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
GL_FUNC(glPopMatrix, void, ());
void glPopMatrix() {
    real_glPopMatrix()();
}
GL_FUNC(glLoadIdentity, void, ());
void glLoadIdentity() {
    real_glLoadIdentity()();
}
GL_FUNC(glScalef, void, (GLfloat x, GLfloat y, GLfloat z));
void glScalef(GLfloat x, GLfloat y, GLfloat z) {
    real_glScalef()(x, y, z);
}
GL_FUNC(glPushMatrix, void, ());
void glPushMatrix() {
    real_glPushMatrix()();
}
GL_FUNC(glDepthMask, void, (GLboolean flag));
void glDepthMask(GLboolean flag) {
    real_glDepthMask()(flag);
}
GL_FUNC(glHint, void, (GLenum target, GLenum mode));
void glHint(GLenum target, GLenum mode) {
    real_glHint()(target, mode);
}
GL_FUNC(glMultMatrixf, void, (const GLfloat *m));
void glMultMatrixf(const GLfloat *m) {
    real_glMultMatrixf()(m);
}
GL_FUNC(glTexCoordPointer, void, (GLint size, GLenum type, GLsizei stride, const void *pointer));
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
    real_glTexCoordPointer()(size, type, stride, pointer);
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
GL_FUNC(glAlphaFunc, void, (GLenum func, GLclampf ref));
void glAlphaFunc(GLenum func, GLclampf ref) {
    real_glAlphaFunc()(func, ref);
}
GL_FUNC(glGetFloatv, void, (GLenum pname, GLfloat *params));
void glGetFloatv(GLenum pname, GLfloat *params) {
    real_glGetFloatv()(pname, params);
}
GL_FUNC(glBindTexture, void, (GLenum target, GLuint texture));
void glBindTexture(GLenum target, GLuint texture) {
    real_glBindTexture()(target, texture);
}
GL_FUNC(glTranslatef, void, (GLfloat x, GLfloat y, GLfloat z));
void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    real_glTranslatef()(x, y, z);
}
GL_FUNC(glShadeModel, void, (GLenum mode));
void glShadeModel(GLenum mode) {
    real_glShadeModel()(mode);
}
GL_FUNC(glOrthof, void, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far));
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    real_glOrthof()(left, right, bottom, top, near, far);
}
GL_FUNC(glDisable, void, (GLenum cap));
void glDisable(GLenum cap) {
    real_glDisable()(cap);
}
GL_FUNC(glCullFace, void, (GLenum mode));
void glCullFace(GLenum mode) {
    real_glCullFace()(mode);
}
GL_FUNC(glRotatef, void, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z));
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    real_glRotatef()(angle, x, y, z);
}
GL_FUNC(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height));
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    real_glViewport()(x, y, width, height);
}
GL_FUNC(glNormal3f, void, (GLfloat nx, GLfloat ny, GLfloat nz));
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
    real_glNormal3f()(nx, ny, nz);
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
GL_FUNC(glGenBuffers, void, (GLsizei n, GLuint *buffers));
void glGenBuffers(GLsizei n, GLuint *buffers) {
    real_glGenBuffers()(n, buffers);
}
