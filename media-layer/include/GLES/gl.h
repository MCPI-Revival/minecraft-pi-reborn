#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define GL_FALSE 0
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_UNSIGNED_BYTE 0x1401
#define GL_FLOAT 0x1406
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_MODELVIEW_MATRIX 0xba6
#define GL_PROJECTION_MATRIX 0xba7
#define GL_VIEWPORT 0xba2
#define GL_DEPTH_TEST 0xb71
#define GL_PACK_ALIGNMENT 0xd05
#define GL_UNPACK_ALIGNMENT 0xcf5
#define GL_SRC_ALPHA 0x302
#define GL_ONE_MINUS_SRC_ALPHA 0x303
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE 0x1702
#define GL_VERTEX_ARRAY 0x8074
#define GL_COLOR_ARRAY 0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_GREATER 0x204
#define GL_ALPHA_TEST 0xbc0
#define GL_TEXTURE_2D 0xde1
#define GL_COLOR_MATERIAL 0xb57
#define GL_PERSPECTIVE_CORRECTION_HINT 0xc50
#define GL_FOG 0xb60
#define GL_LINEAR 0x2601
#define GL_EXP 0x800
#define GL_FOG_DENSITY 0xb62
#define GL_FOG_START 0xb63
#define GL_FOG_END 0xb64
#define GL_FOG_MODE 0xb65
#define GL_FOG_COLOR 0xb66

#include <stdio.h>
#include <stdint.h>

typedef float GLfloat;
typedef float GLclampf;
typedef int GLint;
typedef unsigned char GLboolean;
typedef int GLsizei;
typedef unsigned int GLuint;
typedef ssize_t GLsizeiptr;
typedef intptr_t GLintptr;
typedef int32_t GLfixed;
typedef unsigned int GLbitfield;
typedef unsigned int GLenum;
typedef char GLchar;

void glFogfv(GLenum pname, const GLfloat *params);
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void glLineWidth(GLfloat width);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClear(GLbitfield mask);
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void glFogx(GLenum pname, GLfixed param);
void glFogf(GLenum pname, GLfloat param);
void glMatrixMode(GLenum mode);
void glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void glEnable(GLenum cap);
void glEnableClientState(GLenum array);
void glPolygonOffset(GLfloat factor, GLfloat units);
void glDisableClientState(GLenum array);
void glDepthRangef(GLclampf near, GLclampf far);
void glDepthFunc(GLenum func);
void glBindBuffer(GLenum target, GLuint buffer);
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glPopMatrix();
void glLoadIdentity();
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glPushMatrix();
void glDepthMask(GLboolean flag);
void glHint(GLenum target, GLenum mode);
void glMultMatrixf(const GLfloat *m);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
void glGenTextures(GLsizei n, GLuint *textures);
void glDeleteTextures(GLsizei n, const GLuint *textures);
void glAlphaFunc(GLenum func, GLclampf ref);
void glGetFloatv(GLenum pname, GLfloat *params);
void glBindTexture(GLenum target, GLuint texture);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glShadeModel(GLenum mode);
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
void glDisable(GLenum cap);
void glCullFace(GLenum mode);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
GLboolean glIsEnabled(GLenum cap);
void glGetIntegerv(GLenum pname, GLint *data);
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data);

#ifdef __cplusplus
}
#endif
