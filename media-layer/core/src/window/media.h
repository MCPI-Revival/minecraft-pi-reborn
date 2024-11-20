#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <SDL/SDL.h>
#include <libreborn/libreborn.h>

#include <media-layer/core.h>

// Interactivity
__attribute__((visibility("internal"))) extern bool is_interactable;

// Window
__attribute__((visibility("internal"))) extern GLFWwindow *glfw_window;

// Cursor
__attribute__((visibility("internal"))) void _media_update_cursor();
__attribute__((visibility("internal"))) extern bool ignore_relative_motion;
__attribute__((visibility("internal"))) extern bool raw_mouse_motion_enabled;

// Events
__attribute__((visibility("internal"))) void _media_register_event_listeners();
__attribute__((visibility("internal"))) void _media_handle_media_SDL_PollEvent();
__attribute__((visibility("internal"))) void _media_glfw_motion(GLFWwindow *window, double xpos, double ypos);