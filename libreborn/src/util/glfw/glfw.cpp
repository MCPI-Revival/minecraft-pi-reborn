#include "glfw.h"

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
    glfwWindowHintString(GLFW_X11_CLASS_NAME, id);
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, id);
    // Create Window
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        ERR("Unable To Create GLFW Window");
    }
    // Set Icon
#ifdef _WIN32
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
    // Workaround Segmentation Fault On NVIDIA
    glfwPollEvents();
    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}