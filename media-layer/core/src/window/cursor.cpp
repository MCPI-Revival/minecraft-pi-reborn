#include "media.h"

// Enable/Disable Raw Mouse Motion
bool raw_mouse_motion_enabled = true;
void media_set_raw_mouse_motion_enabled(const bool enabled) {
    raw_mouse_motion_enabled = enabled;
    if (glfw_window) {
        glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }
    if (!raw_mouse_motion_enabled) {
        WARN("Raw mouse motion has been DISABLED, this IS NOT recommended, and should only ever be used on systems that do not support or have broken raw mouse motion.");
    }
}

// Store Cursor State
static bool cursor_grabbed = false;
static bool cursor_visible = true;

// Ignore Relative Cursor Motion
bool ignore_relative_motion = false;

// Update GLFW Cursor State (Client Only)
void _media_update_cursor() {
    if (glfw_window) {
        // Get New State
        const bool new_cursor_visible = is_interactable ? cursor_visible : true;
        const bool new_cursor_grabbed = is_interactable ? cursor_grabbed : false;

        // Store Old Mode
        const int old_mode = glfwGetInputMode(glfw_window, GLFW_CURSOR);

        // Handle Cursor Visibility
        int new_mode;
        if (!new_cursor_visible) {
            if (new_cursor_grabbed) {
                new_mode = GLFW_CURSOR_DISABLED;
            } else {
                new_mode = GLFW_CURSOR_HIDDEN;
            }
        } else {
            new_mode = GLFW_CURSOR_NORMAL;
        }
        if (new_mode != old_mode) {
            // Ignore Relative Cursor Motion When Locking
            if (new_mode == GLFW_CURSOR_DISABLED && old_mode != GLFW_CURSOR_DISABLED) {
                ignore_relative_motion = true;
            }

            // Set New Mode
            glfwSetInputMode(glfw_window, GLFW_CURSOR, new_mode);

            // Handle Cursor Lock/Unlock
            if ((new_mode == GLFW_CURSOR_DISABLED && old_mode != GLFW_CURSOR_DISABLED) || (new_mode != GLFW_CURSOR_DISABLED && old_mode == GLFW_CURSOR_DISABLED)) {
                // Use Raw Mouse Motion
                if (raw_mouse_motion_enabled) {
                    glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, new_mode == GLFW_CURSOR_DISABLED ? GLFW_TRUE : GLFW_FALSE);
                }

                // Request Focus
                glfwRequestWindowAttention(glfw_window);
            }

            // Reset Mouse Position When Unlocking
            if (new_mode != GLFW_CURSOR_DISABLED && old_mode == GLFW_CURSOR_DISABLED) {
                double cursor_x;
                double cursor_y;
                glfwGetCursorPos(glfw_window, &cursor_x, &cursor_y);
                _media_glfw_motion(glfw_window, cursor_x, cursor_y);
            }
        }
    }
}

// Fix SDL Cursor Visibility/Grabbing
SDL_GrabMode media_SDL_WM_GrabInput(const SDL_GrabMode mode) {
    if (mode == SDL_GRAB_QUERY) {
        // Query
        return cursor_grabbed ? SDL_GRAB_ON : SDL_GRAB_OFF;
    } else if (mode == SDL_GRAB_ON) {
        // Store State
        cursor_grabbed = true;
    } else if (mode == SDL_GRAB_OFF) {
        // Store State
        cursor_grabbed = false;
    }
    // Update Cursor GLFW State (Client Only)
    _media_update_cursor();
    // Return
    return mode;
}

// Stub SDL Cursor Visibility
int media_SDL_ShowCursor(const int toggle) {
    if (toggle == SDL_QUERY) {
        // Query
        return cursor_visible ? SDL_ENABLE : SDL_DISABLE;
    } else if (toggle == SDL_ENABLE) {
        // Store State
        cursor_visible = true;
    } else if (toggle == SDL_DISABLE) {
        // Store State
        cursor_visible = false;
    }
    // Update Cursor GLFW State (Client Only)
    _media_update_cursor();
    // Return
    return toggle;
}