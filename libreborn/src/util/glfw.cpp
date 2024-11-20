#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <libreborn/glfw.h>
#include <libreborn/util.h>
#include <libreborn/log.h>
#include <libreborn/config.h>

// Handle GLFW Error
static void glfw_error(__attribute__((unused)) int error, const char *description) {
    WARN("GLFW Error: %s", description);
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
    // App ID
    glfwWindowHintString(GLFW_X11_CLASS_NAME, MCPI_APP_ID);
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, MCPI_APP_ID);
    // Create Window
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        ERR("Unable To Create GLFW Window");
    }
    // Make Window Context Current
    glfwMakeContextCurrent(window);
    // Return
    return window;
}

// Cleanup
void cleanup_glfw(GLFWwindow *window) {
    // Ignore GLFW Errors During Termination
    glfwSetErrorCallback(nullptr);
    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Framebuffer Scaling
void get_glfw_scale(GLFWwindow *window, float *x_scale, float *y_scale) {
    // Get Window Size
    int window_width;
    int window_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0) {
        return;
    }
    // Get Framebuffer Size
    int framebuffer_width;
    int framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    // Calculate Scale
    if (x_scale) {
        *x_scale = float(framebuffer_width) / float(window_width);
    }
    if (y_scale) {
        *y_scale = float(framebuffer_height) / float(window_height);
    }
}