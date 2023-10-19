#pragma once

#include <GLES/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

void glTexSubImage2D_with_scaling(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLsizei normal_texture_width, GLsizei normal_texture_height, GLenum format, GLenum type, const void *pixels);

#ifdef __cplusplus
}
#endif
