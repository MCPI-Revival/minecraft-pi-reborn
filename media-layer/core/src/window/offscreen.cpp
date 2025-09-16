#include "media.h"

#include <GLES/gl.h>

// Offscreen Rendering
static GLFWwindow *offscreen_window = nullptr;
void media_begin_offscreen_render(const int width, const int height) {
    if (!glfw_window) {
        IMPOSSIBLE();
    }
    // Setup GLFW
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
    // Open Window
    offscreen_window = glfwCreateWindow(width, height, "Offscreen Rendering", nullptr, glfw_window);
    if (!offscreen_window) {
        ERR("Unable To Create Offscreen Window");
    }
    // Switch Context
    glfwMakeContextCurrent(offscreen_window);
    media_context_id++;
    // Check Framebuffer Size
    int fb_width;
    int fb_height;
    glfwGetFramebufferSize(offscreen_window, &fb_width, &fb_height);
    if (fb_width != width || fb_height != height) {
        ERR("Offscreen Framebuffer Has Invalid Size");
    }
}
void media_end_offscreen_render() {
    // Destroy Window
    glfwDestroyWindow(offscreen_window);
    offscreen_window = nullptr;
    // Switch Context
    glfwMakeContextCurrent(glfw_window);
    media_context_id++;
}