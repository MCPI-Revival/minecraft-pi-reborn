#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <libreborn/util/glfw.h>
#include <libreborn/util/util.h>
#include <libreborn/log.h>
#include <libreborn/config.h>

// Handle GLFW Error
static void glfw_error(MCPI_UNUSED int error, const char *description) {
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
    const char *id = reborn_config.app.id;
    glfwWindowHintString(GLFW_X11_CLASS_NAME, id);
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, id);
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
    // Workaround Segmentation Fault On NVIDIA
    glfwPollEvents();
    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Framebuffer Scaling
void get_glfw_scale(GLFWwindow *window, float *x_scale_ptr, float *y_scale_ptr) {
    // Output
    float x_scale;
    float y_scale;

    // Default
    x_scale = y_scale = 1.0f;

    // Detect Platform
    if (glfwGetPlatform() == GLFW_PLATFORM_X11) {
        // X11 Has No Scaling
    } else {
        // Get Window Size
        int window_width;
        int window_height;
        glfwGetWindowSize(window, &window_width, &window_height);
        // Get Framebuffer Size
        int framebuffer_width;
        int framebuffer_height;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

        // Calculate
        if (window_width > 0 && window_height > 0) {
            x_scale = float(framebuffer_width) / float(window_width);
            y_scale = float(framebuffer_height) / float(window_height);
        }
    }

    // Return
#define ret(x) if (x##_ptr) *x##_ptr = x
    ret(x_scale);
    ret(y_scale);
#undef ret
}