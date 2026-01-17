#include "internal.h"

// GL_EXT_framebuffer_object
GL_FUNC(glGenFramebuffersEXT, void, (GLsizei n, GLuint *buffers))
void media_glGenFramebuffersEXT(const GLsizei n, GLuint *buffers) {
    real_glGenFramebuffersEXT()(n, buffers);
}
GL_FUNC(glFramebufferTexture2DEXT, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
void media_glFramebufferTexture2DEXT(const GLenum target, const GLenum attachment, const GLenum textarget, const GLuint texture, const GLint level) {
    real_glFramebufferTexture2DEXT()(target, attachment, textarget, texture, level);
}
GL_FUNC(glCheckFramebufferStatusEXT, GLenum, (GLenum target))
GLenum media_glCheckFramebufferStatusEXT(const GLenum target) {
    return real_glCheckFramebufferStatusEXT()(target);
}
GL_FUNC(glBindFramebufferEXT, void, (GLenum target, GLuint framebuffer))
void media_glBindFramebufferEXT(const GLenum target, const GLuint framebuffer) {
    real_glBindFramebufferEXT()(target, framebuffer);
}
GL_FUNC(glDeleteFramebuffersEXT, void, (GLsizei n, const GLuint *buffers))
void media_glDeleteFramebuffersEXT(const GLsizei n, const GLuint *buffers) {
    return real_glDeleteFramebuffersEXT()(n, buffers);
}