#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <SDL/SDL.h>
#include <libreborn/log.h>
#include <libreborn/util/string.h>
#include <libreborn/util/glfw.h>
#include <libreborn/config.h>

#include <media-layer/core.h>

// Interactivity
MCPI_INTERNAL extern bool is_interactable;

// Window
MCPI_INTERNAL extern GLFWwindow *glfw_window;

// Cursor
MCPI_INTERNAL void _media_update_cursor();
MCPI_INTERNAL extern bool ignore_relative_motion;
MCPI_INTERNAL extern bool raw_mouse_motion_enabled;

// Events
MCPI_INTERNAL void _media_register_event_listeners();
MCPI_INTERNAL void _media_handle_media_SDL_PollEvent();
MCPI_INTERNAL void _media_glfw_motion(GLFWwindow *window, double xpos, double ypos);