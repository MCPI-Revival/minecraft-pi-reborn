#pragma once

#include <cstdio>
#include <cstdint>

#define GL_FALSE 0
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ARRAY_BUFFER 0x8892
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
#define GL_DST_ALPHA 0x304
#define GL_ONE_MINUS_SRC_ALPHA 0x303
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE 0x1702
#define GL_VERTEX_ARRAY 0x8074
#define GL_COLOR_ARRAY 0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_NORMAL_ARRAY 0x8075
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
#define GL_BLEND 0xbe2
#define GL_TRIANGLES 0x4
#define GL_TRIANGLE_STRIP 0x5
#define GL_TRIANGLE_FAN 0x6
#define GL_QUADS 0x7
#define GL_FASTEST 0x1101
#define GL_BACK 0x405
#define GL_CULL_FACE 0xb44
#define GL_LEQUAL 0x203
#define GL_EQUAL 0x202
#define GL_ONE_MINUS_DST_COLOR 0x307
#define GL_ONE_MINUS_SRC_COLOR 0x301
#define GL_ZERO 0
#define GL_FLAT 0x1d00
#define GL_SMOOTH 0x1d01
#define GL_SCISSOR_TEST 0xc11
#define GL_TRUE 1
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_SRC_COLOR 0x300
#define GL_DST_COLOR 0x306
#define GL_ONE 1
#define GL_LINES 0x1
#define GL_LINE_STRIP 0x3
#define GL_STATIC_DRAW 0x88e4
#define GL_DYNAMIC_DRAW 0x88e8
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812f
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_NEAREST 0x2600
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_DEPTH_BUFFER_BIT 0x100
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_NO_ERROR 0
#define GL_BYTE 0x1400
#define GL_ACCUM 0x100
#define GL_ALPHA 0x1906
#define GL_NONE 0
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846e
#define GL_LIGHTING 0xb50
#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_RESCALE_NORMAL 0x803a
#define GL_POSITION 0x1203
#define GL_DIFFUSE 0x1201
#define GL_AMBIENT 0x1200
#define GL_SPECULAR 0x1202
#define GL_FRONT_AND_BACK 0x408
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_LIGHT_MODEL_AMBIENT 0xb53
#define GL_STREAM_DRAW 0x88e0
#define GL_UNSIGNED_INT 0x1405
#define GL_COMPILE 0x1300
#define GL_FRAMEBUFFER_EXT 0x8d40
#define GL_COLOR_ATTACHMENT0_EXT 0x8ce0
#define GL_FRAMEBUFFER_COMPLETE_EXT 0x8cd5

typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
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
typedef void GLvoid;

extern "C" {
void media_glFogfv(GLenum pname, const GLfloat *params);
void media_glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void media_glLineWidth(GLfloat width);
void media_glBlendFunc(GLenum sfactor, GLenum dfactor);
void media_glDrawArrays(GLenum mode, GLint first, GLsizei count);
void media_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void media_glClear(GLbitfield mask);
void media_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void media_glFogx(GLenum pname, GLfixed param);
void media_glFogf(GLenum pname, GLfloat param);
void media_glMatrixMode(GLenum mode);
void media_glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void media_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void media_glTexParameteri(GLenum target, GLenum pname, GLint param);
void media_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void media_glEnable(GLenum cap);
void media_glEnableClientState(GLenum array);
void media_glPolygonOffset(GLfloat factor, GLfloat units);
void media_glDisableClientState(GLenum array);
void media_glDepthRangef(GLclampf near, GLclampf far);
void media_glDepthFunc(GLenum func);
void media_glBindBuffer(GLenum target, GLuint buffer);
void media_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void media_glPopMatrix();
void media_glLoadIdentity();
void media_glScalef(GLfloat x, GLfloat y, GLfloat z);
void media_glPushMatrix();
void media_glDepthMask(GLboolean flag);
void media_glHint(GLenum target, GLenum mode);
void media_glMultMatrixf(const GLfloat *m);
void media_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
void media_glDeleteBuffers(GLsizei n, const GLuint *buffers);
void media_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void media_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
void media_glGenTextures(GLsizei n, GLuint *textures);
void media_glDeleteTextures(GLsizei n, const GLuint *textures);
void media_glAlphaFunc(GLenum func, GLclampf ref);
void media_glGetFloatv(GLenum pname, GLfloat *params);
void media_glBindTexture(GLenum target, GLuint texture);
void media_glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void media_glShadeModel(GLenum mode);
void media_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
void media_glDisable(GLenum cap);
void media_glCullFace(GLenum mode);
void media_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void media_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void media_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
GLboolean media_glIsEnabled(GLenum cap);
void media_glGetIntegerv(GLenum pname, GLint *data);
void media_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data);
void media_glGenBuffers(GLsizei n, GLuint *buffers);
GLenum media_glGetError();
void media_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
void media_glPixelStorei(GLenum pname, GLint param);
void media_glNormalPointer(GLenum type, GLsizei stride, const void *pointer);
void media_glLightfv(GLenum light, GLenum pname, const GLfloat *params);
void media_glColorMaterial(GLenum face, GLenum mode);
void media_glLightModelfv(GLenum pname, const GLfloat *params);
GLuint media_glGenLists(GLsizei range);
void media_glDeleteLists(GLuint list, GLsizei range);
void media_glNewList(GLuint list, GLenum mode);
void media_glEndList();
void media_glCallLists(GLsizei n, GLenum type, const GLvoid *lists);
void media_glGenFramebuffersEXT(GLsizei n, GLuint *buffers);
void media_glFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GLenum media_glCheckFramebufferStatusEXT(GLenum target);
void media_glBindFramebufferEXT(GLenum target, GLuint framebuffer);
void media_glDeleteFramebuffersEXT(GLsizei n, const GLuint *buffers);
}