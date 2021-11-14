#include <unistd.h>

#include <SDL/SDL.h>
#include <GLES/gl.h>

#ifndef MCPI_HEADLESS_MODE
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif // #ifndef MCPI_HEADLESS_MODE

#include <libreborn/libreborn.h>
#include <media-layer/core.h>
#include <media-layer/internal.h>

#ifndef MCPI_HEADLESS_MODE
#include "audio/engine.h"
#endif // #ifndef MCPI_HEADLESS_MODE

// Allow Disabling Interaction
static void update_cursor();
static int is_interactable = 1;
void media_set_interactable(int toggle) {
    is_interactable = toggle;
    update_cursor();
}

// GLFW Code Not Needed In Headless Mode
#ifndef MCPI_HEADLESS_MODE

static GLFWwindow *glfw_window;

// Handle GLFW Error
static void glfw_error(__attribute__((unused)) int error, const char *description) {
    WARN("GLFW Error: %s", description);
}

// Pass Character Event
static void character_event(char c) {
    // SDL_UserEvent Is Never Used In MCPI, So It Is Repurposed For Character Events
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = (int) c;
    SDL_PushEvent(&event);
}

// Convert GLFW Key To SDL Key
static SDLKey glfw_key_to_sdl_key(int key) {
    switch (key) {
        // Movement
        case GLFW_KEY_W:
            return SDLK_w;
        case GLFW_KEY_A:
            return SDLK_a;
        case GLFW_KEY_S:
            return SDLK_s;
        case GLFW_KEY_D:
            return SDLK_d;
        case GLFW_KEY_SPACE:
            return SDLK_SPACE;
        case GLFW_KEY_LEFT_SHIFT:
            return SDLK_LSHIFT;
        case GLFW_KEY_RIGHT_SHIFT:
            return SDLK_RSHIFT;
        // Inventory
        case GLFW_KEY_E:
            return SDLK_e;
        // Drop Item
        case GLFW_KEY_Q:
            return SDLK_q;
        // Hotbar
        case GLFW_KEY_1:
            return SDLK_1;
        case GLFW_KEY_2:
            return SDLK_2;
        case GLFW_KEY_3:
            return SDLK_3;
        case GLFW_KEY_4:
            return SDLK_4;
        case GLFW_KEY_5:
            return SDLK_5;
        case GLFW_KEY_6:
            return SDLK_6;
        case GLFW_KEY_7:
            return SDLK_7;
        case GLFW_KEY_8:
            return SDLK_8;
        case GLFW_KEY_9:
             return SDLK_9;
        case GLFW_KEY_0:
             return SDLK_0;
        // UI Control
        case GLFW_KEY_ESCAPE:
            return SDLK_ESCAPE;
        case GLFW_KEY_UP:
            return SDLK_UP;
        case GLFW_KEY_DOWN:
            return SDLK_DOWN;
        case GLFW_KEY_LEFT:
            return SDLK_LEFT;
        case GLFW_KEY_RIGHT:
            return SDLK_RIGHT;
        case GLFW_KEY_TAB:
            return SDLK_TAB;
        case GLFW_KEY_ENTER:
            return SDLK_RETURN;
        case GLFW_KEY_BACKSPACE:
            return SDLK_BACKSPACE;
        // Fullscreen
        case GLFW_KEY_F11:
            return SDLK_F11;
        // Screenshot
        case GLFW_KEY_F2:
            return SDLK_F2;
        // Hide GUI
        case GLFW_KEY_F1:
            return SDLK_F1;
        // Third Person
        case GLFW_KEY_F5:
            return SDLK_F5;
        // Chat
        case GLFW_KEY_T:
            return SDLK_t;
        // Unknown
        default:
            return SDLK_UNKNOWN;
    }
}

// Convert GLFW Key Modifier To SDL Key Modifier
static SDLMod glfw_modifier_to_sdl_modifier(int mods) {
    SDLMod ret = KMOD_NONE;
    // Control
    if ((mods & GLFW_MOD_CONTROL) != 0) {
        ret |= KMOD_CTRL;
    }
    // Shift
    if ((mods & GLFW_MOD_SHIFT) != 0) {
        ret |= KMOD_SHIFT;
    }
    // Alt
    if ((mods & GLFW_MOD_ALT) != 0) {
        ret |= KMOD_ALT;
    }
    // Return
    return ret;
}

// Pass Key Presses To SDL
static void glfw_key(__attribute__((unused)) GLFWwindow *window, int key, int scancode, int action, __attribute__((unused)) int mods) {
    if (is_interactable) {
        SDL_Event event;
        int up = action == GLFW_RELEASE;
        event.type = up ? SDL_KEYUP : SDL_KEYDOWN;
        event.key.state = up ? SDL_RELEASED : SDL_PRESSED;
        event.key.keysym.scancode = scancode;
        event.key.keysym.mod = glfw_modifier_to_sdl_modifier(mods);
        event.key.keysym.sym = glfw_key_to_sdl_key(key);
        SDL_PushEvent(&event);
        if (key == GLFW_KEY_BACKSPACE && !up) {
            character_event((char) '\b');
        }
    }
}

// Pass Text To Minecraft
static void glfw_char(__attribute__((unused)) GLFWwindow *window, unsigned int codepoint) {
    if (is_interactable) {
        character_event((char) codepoint);
    }
}

// Last Mouse Location
static double last_mouse_x = 0;
static double last_mouse_y = 0;
// Ignore Relative Cursor Motion
static int ignore_relative_motion = 0;

// Pass Mouse Movement To SDL
static void glfw_motion(__attribute__((unused)) GLFWwindow *window, double xpos, double ypos) {
    if (is_interactable) {
        SDL_Event event;
        event.type = SDL_MOUSEMOTION;
        event.motion.x = xpos;
        event.motion.y = ypos;
        event.motion.xrel = !ignore_relative_motion ? (xpos - last_mouse_x) : 0;
        event.motion.yrel = !ignore_relative_motion ? (ypos - last_mouse_y) : 0;
        SDL_PushEvent(&event);
    }
    ignore_relative_motion = 0;
    last_mouse_x = xpos;
    last_mouse_y = ypos;
}

// Create And Push SDL Mouse Click Event
static void click_event(int button, int up) {
    if (is_interactable) {
        SDL_Event event;
        event.type = up ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN;
        event.button.x = last_mouse_x;
        event.button.y = last_mouse_y;
        event.button.state = up ? SDL_RELEASED : SDL_PRESSED;
        event.button.button = button;
        SDL_PushEvent(&event);
    }
}

// Pass Mouse Click To SDL
static void glfw_click(__attribute__((unused)) GLFWwindow *window, int button, int action, __attribute__((unused)) int mods) {
    if (is_interactable) {
        int up = action == GLFW_RELEASE;
        int sdl_button = button == GLFW_MOUSE_BUTTON_RIGHT ? SDL_BUTTON_RIGHT : (button == GLFW_MOUSE_BUTTON_LEFT ? SDL_BUTTON_LEFT : SDL_BUTTON_MIDDLE);
        click_event(sdl_button, up);
    }
}

// Pass Mouse Scroll To SDL
static void glfw_scroll(__attribute__((unused)) GLFWwindow *window, __attribute__((unused)) double xoffset, double yoffset) {
    if (is_interactable && yoffset != 0) {
        int sdl_button = yoffset > 0 ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
        click_event(sdl_button, 0);
        click_event(sdl_button, 1);
    }
}

#endif // #ifndef MCPI_HEADLESS_MODE

// Track Media Layer State
static int is_running = 0;

// Disable V-Sync
static int disable_vsync = 0;
void media_disable_vsync() {
    disable_vsync = 1;
#ifndef MCPI_HEADLESS_MODE
    if (is_running) {
        glfwSwapInterval(0);
    }
#endif // #ifndef MCPI_HEADLESS_MODE
}

// Init Media Layer
void SDL_WM_SetCaption(const char *title, __attribute__((unused)) const char *icon) {
    // Don't Enable GLFW In Headless Mode
#ifndef MCPI_HEADLESS_MODE
    // Init GLFW
    glfwSetErrorCallback(glfw_error);

    if (!glfwInit()) {
        ERR("%s", "Unable To Initialize GLFW");
    }

    // Create OpenGL ES 1.1 Context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // Use EGL
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    // Extra Settings
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, 0); // Fix Transparent Window On Wayland

    glfw_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, title, NULL, NULL);
    if (!glfw_window) {
        ERR("%s", "Unable To Create GLFW Window");
    }

    // Don't Process Events In Headless Mode
    glfwSetKeyCallback(glfw_window, glfw_key);
    glfwSetCharCallback(glfw_window, glfw_char);
    glfwSetCursorPosCallback(glfw_window, glfw_motion);
    glfwSetMouseButtonCallback(glfw_window, glfw_click);
    glfwSetScrollCallback(glfw_window, glfw_scroll);

    glfwMakeContextCurrent(glfw_window);

    // Init OpenAL
    _media_audio_init();
#else // #ifndef MCPI_HEADLESS_MODE
    (void) title; // Mark As Used
#endif // #ifndef MCPI_HEADLESS_MODE

    // Set State
    is_running = 1;

    // Update State
    update_cursor();
    if (disable_vsync) {
        media_disable_vsync();
    }
}

void media_swap_buffers() {
#ifndef MCPI_HEADLESS_MODE
    // Don't Swap Buffers In A Context-Less Window
    glfwSwapBuffers(glfw_window);
#endif // #ifndef MCPI_HEADLESS_MODE
}

// Fullscreen Not Needed In Headless Mode
#ifndef MCPI_HEADLESS_MODE
static int is_fullscreen = 0;

// Old Size And Position To Use When Exiting Fullscreen
static int old_width = -1;
static int old_height = -1;
static int old_x = -1;
static int old_y = -1;

// Toggle Fullscreen
void media_toggle_fullscreen() {
    if (is_fullscreen) {
        glfwSetWindowMonitor(glfw_window, NULL, old_x, old_y, old_width, old_height, GLFW_DONT_CARE);

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
#else // #ifndef MCPI_HEADLESS_MODE
void media_toggle_fullscreen() {
}
#endif // #ifndef MCPI_HEADLESS_MODE

// Intercept SDL Events
void _media_handle_SDL_PollEvent() {
    // GLFW And Audio Are Disabled Disabled In Headless Mode
#ifndef MCPI_HEADLESS_MODE
    // Process GLFW Events
    glfwPollEvents();

    // Close Window
    if (glfwWindowShouldClose(glfw_window)) {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
        glfwSetWindowShouldClose(glfw_window, GLFW_FALSE);
    }
#endif // #ifndef MCPI_HEADLESS_MODE
}

// Cleanup Media Layer
void media_cleanup() {
    if (is_running) {
        // GLFW And Audio Are Disabled In Headless Mode
#ifndef MCPI_HEADLESS_MODE
        // Ignore GLFW Errors During Termination
        glfwSetErrorCallback(NULL);

        // Terminate GLFW
        glfwDestroyWindow(glfw_window);
        glfwTerminate();

        // Cleanup OpenAL
        _media_audio_cleanup();
#endif // #ifndef MCPI_HEADLESS_MODE

        // Update State
        is_running = 0;
    }
}
// Always Cleanup Media Layer
__attribute__((destructor)) static void always_cleanup() {
    media_cleanup();
}

// Store Cursor State
static int cursor_grabbed = 0;
static int cursor_visible = 1;

// Update GLFW Cursor State (Client Only)
static void update_cursor() {
#ifndef MCPI_HEADLESS_MODE
    if (is_running) {
        // Get New State
        int new_cursor_visible = is_interactable ? cursor_visible : 1;
        int new_cursor_grabbed = is_interactable ? cursor_grabbed : 0;

        // Store Old Mode
        int old_mode = glfwGetInputMode(glfw_window, GLFW_CURSOR);

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
                ignore_relative_motion = 1;
            }

            // Set New Mode
            glfwSetInputMode(glfw_window, GLFW_CURSOR, new_mode);

            // Handle Cursor Lock/Unlock
            if ((new_mode == GLFW_CURSOR_DISABLED && old_mode != GLFW_CURSOR_DISABLED) || (new_mode != GLFW_CURSOR_DISABLED && old_mode == GLFW_CURSOR_DISABLED)) {
                // Use Raw Mouse Motion
                glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, new_mode == GLFW_CURSOR_DISABLED ? GLFW_TRUE : GLFW_FALSE);
            }

            // Reset Mouse Position When Unlocking
            if (new_mode != GLFW_CURSOR_DISABLED && old_mode == GLFW_CURSOR_DISABLED) {
                double cursor_x;
                double cursor_y;
                glfwGetCursorPos(glfw_window, &cursor_x, &cursor_y);
                glfw_motion(glfw_window, cursor_x, cursor_y);
            }
        }
    }
#endif // #ifndef MCPI_HEADLESS_MODE
}

// Fix SDL Cursor Visibility/Grabbing
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode) {
    if (mode == SDL_GRAB_QUERY) {
        // Query
        return cursor_grabbed ? SDL_GRAB_ON : SDL_GRAB_OFF;
    } else if (mode == SDL_GRAB_ON) {
        // Store State
        cursor_grabbed = 1;
    } else if (mode == SDL_GRAB_OFF) {
        // Store State
        cursor_grabbed = 0;
    }
    // Update Cursor GLFW State (Client Only)
    update_cursor();
    // Return
    return mode;
}

// Stub SDL Cursor Visibility
int SDL_ShowCursor(int toggle) {
    if (toggle == SDL_QUERY) {
        // Query
        return cursor_visible ? SDL_ENABLE : SDL_DISABLE;
    } else if (toggle == SDL_ENABLE) {
        // Store State
        cursor_visible = 1;
    } else if (toggle == SDL_DISABLE) {
        // Store State
        cursor_visible = 0;
    }
    // Update Cursor GLFW State (Client Only)
    update_cursor();
    // Return
    return toggle;
}

// Get Framebuffer Size
void media_get_framebuffer_size(int *width, int *height) {
#ifndef MCPI_HEADLESS_MODE
    if (glfw_window != NULL) {
        glfwGetFramebufferSize(glfw_window, width, height);
        return;
    }
#endif // #ifndef MCPI_HEADLESS_MODE
    *width = DEFAULT_WIDTH;
    *height = DEFAULT_HEIGHT;
}
