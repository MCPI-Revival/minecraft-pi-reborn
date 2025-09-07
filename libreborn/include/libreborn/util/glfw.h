#pragma once

// GLFW Helpers
#ifndef GLFW_VERSION_MAJOR
#error "Missing GLFW"
#endif

void init_glfw();
GLFWwindow *create_glfw_window(const char *title, int width, int height);
void cleanup_glfw(GLFWwindow *window);
void get_glfw_scale(GLFWwindow *window, float *x_scale_ptr, float *y_scale_ptr);