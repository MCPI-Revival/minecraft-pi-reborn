#include <ctime>
#include <unistd.h>

#include "media.h"
#include "../audio/audio.h"

// Window
GLFWwindow *glfw_window = nullptr;

// Disable V-Sync
static bool disable_vsync = false;
void media_disable_vsync() {
    disable_vsync = true;
    if (glfw_window) {
        glfwSwapInterval(0);
    }
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
    init_glfw();

    // Create OpenGL Context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    // Extra Settings
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);

    // Create Window
    glfw_window = create_glfw_window(title, DEFAULT_WIDTH, DEFAULT_HEIGHT);

    // Event Handlers
    _media_register_event_listeners();

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
        // Terminate GLFW
        cleanup_glfw(glfw_window);

        // Cleanup OpenAL
        _media_audio_cleanup();

        // Mark As Stopped
        glfw_window = nullptr;
    }
}