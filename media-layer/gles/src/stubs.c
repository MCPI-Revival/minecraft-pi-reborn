#include <GLES/gl.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void glFogfv(GLenum pname, const GLfloat *params) {
}
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
}
void glLineWidth(GLfloat width) {
}
void glBlendFunc(GLenum sfactor, GLenum dfactor) {
}
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
}
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
}
void glClear(GLbitfield mask) {
}
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
}
void glFogx(GLenum pname, GLfixed param) {
}
void glFogf(GLenum pname, GLfloat param) {
}
void glMatrixMode(GLenum mode) {
}
void glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
}
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
}
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
}
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {
}
void glEnable(GLenum cap) {
}
void glEnableClientState(GLenum array) {
}
void glPolygonOffset(GLfloat factor, GLfloat units) {
}
void glDisableClientState(GLenum array) {
}
void glDepthRangef(GLclampf near, GLclampf far) {
}
void glDepthFunc(GLenum func) {
}
void glBindBuffer(GLenum target, GLuint buffer) {
}
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
}
void glPopMatrix() {
}
void glLoadIdentity() {
}
void glScalef(GLfloat x, GLfloat y, GLfloat z) {
}
void glPushMatrix() {
}
void glDepthMask(GLboolean flag) {
}
void glHint(GLenum target, GLenum mode) {
}
void glMultMatrixf(const GLfloat *m) {
}
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {
}
void glDeleteBuffers(GLsizei n, const GLuint *buffers) {
}
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
}
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
}
void glGenTextures(GLsizei n, GLuint *textures) {
    static int i = 0;
    for (int j = 0; j < n; j++) {
        textures[j] = i++;
    }
}
void glDeleteTextures(GLsizei n, const GLuint *textures) {
}
void glAlphaFunc(GLenum func, GLclampf ref) {
}
void glGetFloatv(GLenum pname, GLfloat *params) {
    switch (pname) {
        case GL_MODELVIEW_MATRIX:
        case GL_PROJECTION_MATRIX: {
            params[0] = 1;
            params[1] = 0;
            params[2] = 0;
            params[3] = 0;
            params[4] = 0;
            params[5] = 1;
            params[6] = 0;
            params[7] = 0;
            params[8] = 0;
            params[9] = 0;
            params[10] = 1;
            params[11] = 0;
            params[12] = 0;
            params[13] = 0;
            params[14] = 0;
            params[15] = 1;
            break;
        }
        default: {
            params[0] = 0;
            break;
        }
    }
}
static GLuint current_texture = 0;
void glBindTexture(GLenum target, GLuint texture) {
    if (target == GL_TEXTURE_2D) {
        current_texture = texture;
    }
}
void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
}
void glShadeModel(GLenum mode) {
}
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
}
void glDisable(GLenum cap) {
}
void glCullFace(GLenum mode) {
}
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
}
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
}
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
}
GLboolean glIsEnabled(GLenum cap) {
    return GL_FALSE;
}
void glGetIntegerv(GLenum pname, GLint *data) {
    switch (pname) {
        case GL_TEXTURE_BINDING_2D: {
            data[0] = current_texture;
            break;
        }
        case GL_UNPACK_ALIGNMENT: {
            data[0] = 1;
            break;
        }
        default: {
            data[0] = 0;
            break;
        }
    }
}
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data) {
}

#pragma GCC diagnostic pop
