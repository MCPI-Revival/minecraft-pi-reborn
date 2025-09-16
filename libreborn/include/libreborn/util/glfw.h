#pragma once

// GLFW Helpers
#ifndef GLFW_VERSION_MAJOR
#error "Missing GLFW"
#endif

void init_glfw();
GLFWwindow *create_glfw_window(const char *title, int width, int height);
void cleanup_glfw(GLFWwindow *window);