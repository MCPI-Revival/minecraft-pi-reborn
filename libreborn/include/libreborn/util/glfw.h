#pragma once

// GLFW Helpers
#ifndef GLFW_VERSION_MAJOR
#error "Missing GLFW"
#endif

// Initialize GLFW
MCPI_REBORN_UTIL_PUBLIC void init_glfw();

// Create Window
MCPI_REBORN_UTIL_PUBLIC GLFWwindow *create_glfw_window(const char *title, int width, int height);

// Close Window And Uninitialized GLFW
MCPI_REBORN_UTIL_PUBLIC void cleanup_glfw(GLFWwindow *window);