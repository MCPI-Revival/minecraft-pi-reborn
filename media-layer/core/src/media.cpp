#include <ctime>
#include <unistd.h>

#include <SDL/SDL.h>
#include <libreborn/libreborn.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <media-layer/core.h>
#include <media-layer/internal.h>

#include "audio/engine.h"

// Allow Disabling Interaction
static void update_cursor();
static int is_interactable = 1;
void media_set_interactable(int toggle) {
    if (bool(toggle) != is_interactable) {
        is_interactable = toggle;
        update_cursor();
    }
}

// Store Cursor State
static bool cursor_grabbed = false;
static bool cursor_visible = true;

// Track If Raw Mouse Motion Is Enabled
static bool raw_mouse_motion_enabled = true;

// Window
static GLFWwindow *glfw_window = nullptr;

// Handle GLFW Error
static void glfw_error(__attribute__((unused)) int error, const char *description) {
    WARN("GLFW Error: %s", description);
}

// Convert GLFW Key To SDL Key
static SDLKey glfw_key_to_sdl_key(const int key) {
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
        // Toolbar
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
        case GLFW_KEY_DELETE:
            return SDLK_DELETE;
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
static SDLMod glfw_modifier_to_sdl_modifier(const int mods) {
    int ret = KMOD_NONE;
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
    return SDLMod(ret);
}

// Pass Key Presses To SDL
static void glfw_key_raw(int key, int scancode, int action, int mods) {
    SDL_Event event1;
    bool up = action == GLFW_RELEASE;
    event1.type = up ? SDL_KEYUP : SDL_KEYDOWN;
    event1.key.state = up ? SDL_RELEASED : SDL_PRESSED;
    event1.key.keysym.scancode = scancode;
    event1.key.keysym.mod = glfw_modifier_to_sdl_modifier(mods);
    event1.key.keysym.sym = glfw_key_to_sdl_key(key);
    SDL_PushEvent(&event1);
    // Allow MCPI To Access Original GLFW Keycode
    SDL_Event event2;
    event2.type = SDL_USEREVENT;
    event2.user.code = USER_EVENT_REAL_KEY;
    event2.user.data1 = event1.key.state;
    event2.user.data2 = key;
    SDL_PushEvent(&event2);
}
static void glfw_key(__attribute__((unused)) GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    if (is_interactable) {
        glfw_key_raw(key, scancode, action, mods);
    }
}

// Pass Text To Minecraft
static void character_event(char c) {
    if (!is_interactable) {
        return;
    }
    // SDL_UserEvent Is Never Used In MCPI, So It Is Repurposed For Character Events
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = USER_EVENT_CHARACTER;
    event.user.data1 = (int) c;
    SDL_PushEvent(&event);
}
static void codepoint_to_utf8(unsigned char *const buffer, const unsigned int code) {
    // https://stackoverflow.com/a/42013433/16198887
    if (code <= 0x7f) {
        buffer[0] = code;
    } else if (code <= 0x7ff) {
        buffer[0] = 0xc0 | (code >> 6); // 110xxxxx
        buffer[1] = 0x80 | (code & 0x3f); // 10xxxxxx
    } else if (code <= 0xffff) {
        buffer[0] = 0xe0 | (code >> 12); // 1110xxxx
        buffer[1] = 0x80 | ((code >> 6) & 0x3f); // 10xxxxxx
        buffer[2] = 0x80 | (code & 0x3f); // 10xxxxxx
    } else if (code <= 0x10ffff) {
        buffer[0] = 0xf0 | (code >> 18); // 11110xxx
        buffer[1] = 0x80 | ((code >> 12) & 0x3f); // 10xxxxxx
        buffer[2] = 0x80 | ((code >> 6) & 0x3f); // 10xxxxxx
        buffer[3] = 0x80 | (code & 0x3f); // 10xxxxxx
    }
}
static void glfw_char(__attribute__((unused)) GLFWwindow *window, const unsigned int codepoint) {
    // Convert
    size_t str_size = 4 /* Maximum UTF-8 character size */ + 1 /* NULL-terminator */;
    char str[str_size] = {};
    codepoint_to_utf8((unsigned char *) str, codepoint);
    char *cp437_str = to_cp437(str);
    // Send Event
    for (int i = 0; cp437_str[i] != '\0'; i++) {
        character_event(cp437_str[i]);
    }
    // Free
    free(cp437_str);
}

// Last Mouse Location
static double last_mouse_x = 0;
static double last_mouse_y = 0;
// Ignore Relative Cursor Motion
static bool ignore_relative_motion = false;

// Convert Screen Coordinates To Pixels
static void convert_to_pixels(GLFWwindow *window, double *xpos, double *ypos) {
    // Skip If Cursor Is Grabbed
    if (cursor_grabbed && raw_mouse_motion_enabled) {
        return;
    }
    // Get Window Size
    int window_width;
    int window_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    // Get Framebuffer Size
    int framebuffer_width;
    int framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    // Calculate Ratios
    const double width_ratio = ((double) framebuffer_width) / ((double) window_width);
    const double height_ratio = ((double) framebuffer_height) / ((double) window_height);
    // Multiply
    *xpos *= width_ratio;
    *ypos *= height_ratio;
}

// Pass Mouse Movement To SDL
static void glfw_motion(__attribute__((unused)) GLFWwindow *window, double xpos, double ypos) {
    convert_to_pixels(window, &xpos, &ypos);
    if (is_interactable) {
        SDL_Event event;
        event.type = SDL_MOUSEMOTION;
        event.motion.x = xpos;
        event.motion.y = ypos;
        event.motion.xrel = !ignore_relative_motion ? (xpos - last_mouse_x) : 0;
        event.motion.yrel = !ignore_relative_motion ? (ypos - last_mouse_y) : 0;
        SDL_PushEvent(&event);
    }
    ignore_relative_motion = false;
    last_mouse_x = xpos;
    last_mouse_y = ypos;
}

// Create And Push SDL Mouse Click Event
static void click_event(int button, bool up) {
    SDL_Event event;
    event.type = up ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN;
    event.button.x = last_mouse_x;
    event.button.y = last_mouse_y;
    event.button.state = up ? SDL_RELEASED : SDL_PRESSED;
    event.button.button = button;
    SDL_PushEvent(&event);
}

// Pass Mouse Click To SDL
static void glfw_click_raw(const int button, const int action) {
    const bool up = action == GLFW_RELEASE;
    const int sdl_button = button == GLFW_MOUSE_BUTTON_RIGHT ? SDL_BUTTON_RIGHT : (button == GLFW_MOUSE_BUTTON_LEFT ? SDL_BUTTON_LEFT : SDL_BUTTON_MIDDLE);
    click_event(sdl_button, up);
}
static void glfw_click(__attribute__((unused)) GLFWwindow *window, const int button, const int action, __attribute__((unused)) int mods) {
    if (is_interactable) {
        glfw_click_raw(button, action);
    }
}

// Pass Mouse Scroll To SDL
static void glfw_scroll(__attribute__((unused)) GLFWwindow *window, __attribute__((unused)) double xoffset, double yoffset) {
    if (is_interactable && yoffset != 0) {
        int sdl_button = yoffset > 0 ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
        click_event(sdl_button, false);
        click_event(sdl_button, true);
    }
}

// Enable/Disable Raw Mouse Motion
void media_set_raw_mouse_motion_enabled(const int enabled) {
    raw_mouse_motion_enabled = enabled;
    if (glfw_window) {
        glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }
    if (!raw_mouse_motion_enabled) {
        WARN("Raw mouse motion has been DISABLED, this IS NOT recommended, and should only ever be used on systems that don't support or have broken raw mouse motion.");
    }
}

// Disable V-Sync
static int disable_vsync = 0;
void media_disable_vsync() {
    disable_vsync = 1;
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
#ifdef MCPI_USE_GLES1_COMPATIBILITY_LAYER
extern "C" void init_gles_compatibility_layer(void *);
#endif
void SDL_WM_SetCaption(const char *title, __attribute__((unused)) const char *icon) {
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

    // Create OpenGL ES Context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#ifdef MCPI_USE_GLES1_COMPATIBILITY_LAYER
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif
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
    glfw_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, title, NULL, NULL);
    if (!glfw_window) {
        ERR("Unable To Create GLFW Window");
    }

    // Event Handlers
    glfwSetKeyCallback(glfw_window, glfw_key);
    glfwSetCharCallback(glfw_window, glfw_char);
    glfwSetCursorPosCallback(glfw_window, glfw_motion);
    glfwSetMouseButtonCallback(glfw_window, glfw_click);
    glfwSetScrollCallback(glfw_window, glfw_scroll);

    // Make Window Context Current
    glfwMakeContextCurrent(glfw_window);

    // Setup Compatibility Layer
#ifdef MCPI_USE_GLES1_COMPATIBILITY_LAYER
    init_gles_compatibility_layer((void *) glfwGetProcAddress);
#endif

    // Debug
    glGetString_t glGetString = (glGetString_t) glfwGetProcAddress("glGetString");
    DEBUG("Using %s", (*glGetString)(GL_VERSION));

    // Init OpenAL
    _media_audio_init();

    // Update State
    update_cursor();
    if (disable_vsync) {
        media_disable_vsync();
    }

    // Always Cleanup Media Layer
    atexit(media_cleanup);
}

void media_swap_buffers() {
    if (glfw_window) {
        glfwSwapBuffers(glfw_window);
    }
}

// Track Fullscreen
static bool is_fullscreen = false;
// Old Size And Position To Use When Exiting Fullscreen
static int old_width = -1;
static int old_height = -1;
static int old_x = -1;
static int old_y = -1;

// Toggle Fullscreen
void media_toggle_fullscreen() {
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

// Intercept SDL Events
void _media_handle_SDL_PollEvent() {
    if (glfw_window) {
        // Process GLFW Events
        glfwPollEvents();
        // Close Window
        if (glfwWindowShouldClose(glfw_window)) {
            SDL_Event event;
            event.type = SDL_QUIT;
            SDL_PushEvent(&event);
            glfwSetWindowShouldClose(glfw_window, GLFW_FALSE);
        }
    }
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
    }
}

// Update GLFW Cursor State (Client Only)
static void update_cursor() {
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
                if (!glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED)) {
                    glfwRequestWindowAttention(glfw_window);
                }
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
}

// Fix SDL Cursor Visibility/Grabbing
SDL_GrabMode SDL_WM_GrabInput(const SDL_GrabMode mode) {
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
    update_cursor();
    // Return
    return mode;
}

// Stub SDL Cursor Visibility
int SDL_ShowCursor(const int toggle) {
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
    update_cursor();
    // Return
    return toggle;
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
