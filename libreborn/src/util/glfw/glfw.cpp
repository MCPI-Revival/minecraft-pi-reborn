#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <libreborn/util/util.h>
#include <libreborn/config.h>

#include "glfw.h"

// Handle GLFW Error
static void glfw_error(const int error, const char *description) {
    WARN("GLFW Error: %i: %s", error, description);
}

// Init
void init_glfw() {
    reborn_check_display();
    glfwSetErrorCallback(glfw_error);
    if (!glfwInit()) {
        ERR("Unable To Initialize GLFW");
    }
}

// Create Window
GLFWwindow *create_glfw_window(const char *title, const int width, const int height) {
    // Scaling
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
    // App ID
    const char *id = reborn_config.app.id;
    _reborn_set_app_id_global(id);
    // Create Window
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        const char *err = "Unable To Create GLFW Window";
#ifdef _WIN32
        MessageBoxA(nullptr, err, title, MB_ICONERROR | MB_OK);
#endif
        ERR("%s", err);
    }
    // Configure Window
#ifdef _WIN32
    _reborn_set_app_id_and_relaunch_behavior(window, id);
    _reborn_set_window_icon(window);
#endif
    // Make Window Context Current
    glfwMakeContextCurrent(window);
    // Return
    return window;
}

// Cleanup
void cleanup_glfw(GLFWwindow *window) {
    // Ignore GLFW Errors During Termination
    glfwSetErrorCallback(nullptr);
    // Clear Properties
#ifdef _WIN32
    _reborn_free_window_properties(window);
#endif
    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}