#pragma once

#include <GLES/gl.h>

struct Texture;

extern "C" {
void media_glTexSubImage2D_with_scaling(const Texture *target, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLsizei normal_texture_width, GLsizei normal_texture_height, const void *pixels);
void textures_add(int width, int height, const unsigned char *pixels);
}