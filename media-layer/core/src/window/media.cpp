#include <ctime>
#include <unistd.h>

#include "media.h"
#include "../audio/engine.h"

// Window
GLFWwindow *glfw_window = nullptr;

// Handle GLFW Error
static void glfw_error(__attribute__((unused)) int error, const char *description) {
    WARN("GLFW Error: %s", description);
}

// Disable V-Sync
static bool disable_vsync = false;
void media_disable_vsync() {
    disable_vsync = true;
    if (glfw_window) {
        glfwSwapInterval(0);
    }
}

// Force EGL
static int force_egl = 0;
void media_force_egl() {
    if (force_egl == -1) {
        IMPOSSIBLE();
    }
    force_egl = 1;
}

// Init Media Layer
#define GL_VERSION 0x1f02
typedef const char *(*glGetString_t)(unsigned int name);
void media_SDL_WM_SetCaption(const char *title, __attribute__((unused)) const char *icon) {
    // Disable In Headless Mode
    if (reborn_is_headless()) {
        return;
    }

    // Init GLFW
    reborn_check_display();
    glfwSetErrorCallback(glfw_error);
    if (!glfwInit()) {
        ERR("Unable To Initialize GLFW");
    }

    // Create OpenGL Context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    // Use EGL
    if (force_egl) {
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    }
    force_egl = -1;
    // Extra Settings
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, 0); // Fix Transparent Window On Wayland
    // App ID
    glfwWindowHintString(GLFW_X11_CLASS_NAME, MCPI_APP_ID);
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, MCPI_APP_ID);

    // Create Window
    glfw_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, title, nullptr, nullptr);
    if (!glfw_window) {
        ERR("Unable To Create GLFW Window");
    }

    // Event Handlers
    _media_register_event_listeners();

    // Make Window Context Current
    glfwMakeContextCurrent(glfw_window);

    // Debug
    const glGetString_t glGetString = (glGetString_t) glfwGetProcAddress("glGetString");
    DEBUG("Using OpenGL %s", (*glGetString)(GL_VERSION));

    // Init OpenAL
    _media_audio_init();

    // Update State
    _media_update_cursor();
    if (disable_vsync) {
        media_disable_vsync();
    }

    // Always Cleanup Media Layer
    atexit(media_cleanup);
}

// Cleanup Media Layer
void media_cleanup() {
    if (glfw_window) {
        // Ignore GLFW Errors During Termination
        glfwSetErrorCallback(nullptr);

        // Terminate GLFW
        glfwDestroyWindow(glfw_window);
        glfwTerminate();

        // Cleanup OpenAL
        _media_audio_cleanup();

        // Mark As Stopped
        glfw_window = nullptr;
    }
}