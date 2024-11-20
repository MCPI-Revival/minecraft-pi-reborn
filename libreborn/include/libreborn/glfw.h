#pragma once

// GLFW Helpers
#ifdef GLFW_VERSION_MAJOR

void init_glfw();
GLFWwindow *create_glfw_window(const char *title, int width, int height);
void cleanup_glfw(GLFWwindow *window);
void get_glfw_scale(GLFWwindow *window, float *x_scale, float *y_scale);

#endif