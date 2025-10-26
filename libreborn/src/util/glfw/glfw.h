#pragma once

#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <libreborn/log.h>
#include <libreborn/util/glfw.h>

// Set Window Icon
#ifdef _WIN32
MCPI_INTERNAL void _reborn_set_window_icon(GLFWwindow *window);
#endif

// Set App ID
MCPI_INTERNAL void _reborn_set_app_id_global(const std::string &id);
#ifdef _WIN32
MCPI_INTERNAL void _reborn_set_app_id_and_relaunch_behavior(GLFWwindow *window, const std::string &id);
MCPI_INTERNAL void _reborn_free_window_properties(GLFWwindow *window);
#endif