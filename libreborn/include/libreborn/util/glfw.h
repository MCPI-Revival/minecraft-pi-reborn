#pragma once

// GLFW Helpers
#ifndef GLFW_VERSION_MAJOR
#error "Missing GLFW"
#endif

// Initialize GLFW
void init_glfw();

// Create Window
GLFWwindow *create_glfw_window(const char *title, int width, int height);

// Close Window And Uninitialized GLFW
void cleanup_glfw(GLFWwindow *window);