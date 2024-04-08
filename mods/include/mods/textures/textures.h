#pragma once

#include <GLES/gl.h>

extern "C" {
void glTexSubImage2D_with_scaling(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLsizei normal_texture_width, GLsizei normal_texture_height, GLenum format, GLenum type, const void *pixels);
}