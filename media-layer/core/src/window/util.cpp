#include "media.h"

// Allow Disabling Interaction
bool is_interactable = true;
void media_set_interactable(const int toggle) {
    if (bool(toggle) != is_interactable) {
        is_interactable = toggle;
        _media_update_cursor();
    }
}

// Get Framebuffer Size
void media_get_framebuffer_size(int *width, int *height) {
    if (glfw_window) {
        glfwGetFramebufferSize(glfw_window, width, height);
    } else {
        *width = DEFAULT_WIDTH;
        *height = DEFAULT_HEIGHT;
    }
}

// Check OpenGL Extension
int media_has_extension(const char *name) {
    if (glfw_window) {
        return glfwExtensionSupported(name);
    } else {
        return 0;
    }
}

// Swap Buffers
void media_swap_buffers() {
    if (glfw_window) {
        glfwSwapBuffers(glfw_window);
    }
}

// Toggle Fullscreen
void media_toggle_fullscreen() {
    // Track Fullscreen
    static bool is_fullscreen = false;
    // Old Size And Position To Use When Exiting Fullscreen
    static int old_width = -1;
    static int old_height = -1;
    static int old_x = -1;
    static int old_y = -1;
    // Run
    if (glfw_window) {
        if (is_fullscreen) {
            glfwSetWindowMonitor(glfw_window, nullptr, old_x, old_y, old_width, old_height, GLFW_DONT_CARE);

            old_width = -1;
            old_height = -1;
            old_x = -1;
            old_y = -1;
        } else {
            glfwGetWindowSize(glfw_window, &old_width, &old_height);
            glfwGetWindowPos(glfw_window, &old_x, &old_y);

            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);

            glfwSetWindowMonitor(glfw_window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
        }
        is_fullscreen = !is_fullscreen;
    }
}