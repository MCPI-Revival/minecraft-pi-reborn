#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <libreborn/log.h>
#include <libreborn/util/glfw.h>

// Set Window Icon
#ifdef _WIN32
MCPI_INTERNAL void _reborn_set_window_icon(GLFWwindow *window);
#endif