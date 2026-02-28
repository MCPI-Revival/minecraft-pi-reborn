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
extern bool is_interactable;

// Window
extern GLFWwindow *glfw_window;

// Cursor
void _media_update_cursor();
extern bool ignore_relative_motion;
extern bool raw_mouse_motion_enabled;

// Events
void _media_register_event_listeners();
void _media_handle_media_SDL_PollEvent();
void _media_glfw_motion(GLFWwindow *window, double xpos, double ypos);

// Texture Download
void _media_cancel_download(unsigned int texture_id);

// Clean Up Offscreen Rendering
void _media_cleanup_offscreen_render();