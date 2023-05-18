#include <unistd.h>

#include <SDL/SDL.h>
#include <libreborn/libreborn.h>

#ifndef MCPI_HEADLESS_MODE
#include <time.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <media-layer/core.h>
#include <media-layer/internal.h>

#ifndef MCPI_HEADLESS_MODE
#include "audio/engine.h"
#endif

// Allow Disabling Interaction
static void update_cursor();
#ifndef MCPI_HEADLESS_MODE
static void emit_events_after_is_interactable_change();
#endif
static int is_interactable = 1;
void media_set_interactable(int toggle) {
    if (toggle != is_interactable) {
        is_interactable = toggle;
        update_cursor();
#ifndef MCPI_HEADLESS_MODE
        emit_events_after_is_interactable_change();
#endif
    }
}

// Track Media Layer State
static volatile int is_running = 0;

// Store Cursor State
static int cursor_grabbed = 0;
static int cursor_visible = 1;

// GLFW Code Not Needed In Headless Mode
#ifndef MCPI_HEADLESS_MODE

static GLFWwindow *glfw_window = NULL;

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
#define IMAGINARY_GLFW_CRAFTING_KEY GLFW_KEY_LAST
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
        // Crafting
        case IMAGINARY_GLFW_CRAFTING_KEY:
            return SDLK_WORLD_0;
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
static void glfw_key_raw(int key, __attribute__((unused)) int scancode, int action, int mods) {
    SDL_Event event;
    int up = action == GLFW_RELEASE;
    event.type = up ? SDL_KEYUP : SDL_KEYDOWN;
    event.key.state = up ? SDL_RELEASED : SDL_PRESSED;
    event.key.keysym.scancode = key; // Allow MCPI To Access Original GLFW Keycode
    event.key.keysym.mod = glfw_modifier_to_sdl_modifier(mods);
    event.key.keysym.sym = glfw_key_to_sdl_key(key);
    SDL_PushEvent(&event);
}
static void glfw_key(__attribute__((unused)) GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (is_interactable) {
        glfw_key_raw(key, scancode, action, mods);
    }
}

// Pass Text To Minecraft
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
static void glfw_char(__attribute__((unused)) GLFWwindow *window, unsigned int codepoint) {
    if (is_interactable) {
        // Convert
        size_t str_size = 4 /* Maximum UTF-8 character size */ + 1 /* NULL-terminator */;
        char str[str_size];
        memset(str, 0, str_size);
        codepoint_to_utf8((unsigned char *) str, codepoint);
        char *cp437_str = to_cp437(str);
        // Send Event·
        for (int i = 0; cp437_str[i] != '\0'; i++) {
            character_event(cp437_str[i]);
        }
        // Free
        free(cp437_str);
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
    SDL_Event event;
    event.type = up ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN;
    event.button.x = last_mouse_x;
    event.button.y = last_mouse_y;
    event.button.state = up ? SDL_RELEASED : SDL_PRESSED;
    event.button.button = button;
    SDL_PushEvent(&event);
}

// Pass Mouse Click To SDL
static void glfw_click_raw(int button, int action) {
    int up = action == GLFW_RELEASE;
    int sdl_button = button == GLFW_MOUSE_BUTTON_RIGHT ? SDL_BUTTON_RIGHT : (button == GLFW_MOUSE_BUTTON_LEFT ? SDL_BUTTON_LEFT : SDL_BUTTON_MIDDLE);
    click_event(sdl_button, up);
}
static void glfw_click(__attribute__((unused)) GLFWwindow *window, int button, int action, __attribute__((unused)) int mods) {
    if (is_interactable) {
        glfw_click_raw(button, action);
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

// Controller Events
static SDLKey glfw_controller_button_to_key(int button) {
    switch (button) {
        // Jump
        case GLFW_GAMEPAD_BUTTON_A:
            return GLFW_KEY_SPACE;
        // Drop Item
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
            return GLFW_KEY_Q;
        // Inventory
        case GLFW_GAMEPAD_BUTTON_Y:
            return GLFW_KEY_E;
        // Third-Person
        case GLFW_GAMEPAD_BUTTON_DPAD_UP:
            return GLFW_KEY_F5;
        // Sneak
        case GLFW_GAMEPAD_BUTTON_B:
            return GLFW_KEY_LEFT_SHIFT;
        // Chat
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
            return GLFW_KEY_T;
        // Pause
        case GLFW_GAMEPAD_BUTTON_START:
        case GLFW_GAMEPAD_BUTTON_BACK:
            return GLFW_KEY_ESCAPE;
        // Crafting
        case GLFW_GAMEPAD_BUTTON_X:
            return IMAGINARY_GLFW_CRAFTING_KEY;
        // Unknown
        default:
            return GLFW_KEY_UNKNOWN;
    }
}
static void glfw_controller_button(int button, int action) {
    int key = glfw_controller_button_to_key(button);
    if (key != GLFW_KEY_UNKNOWN) {
        // Press Key
        glfw_key_raw(key, glfwGetKeyScancode(key), action, 0);
    } else {
        // Scrolling
        if (button == GLFW_GAMEPAD_BUTTON_LEFT_BUMPER) {
            key = SDL_BUTTON_WHEELUP;
        } else if (button == GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER) {
            key = SDL_BUTTON_WHEELDOWN;
        }
        if (key != GLFW_KEY_UNKNOWN) {
            click_event(key, action == GLFW_PRESS);
        }
    }
}

// Controller Movement Axis
static int controller_horizontal_key = GLFW_KEY_UNKNOWN;
static int controller_vertical_key = GLFW_KEY_UNKNOWN;
static void release_and_press_key(int *old_key, int new_key) {
    if (*old_key != new_key) {
        if (*old_key != GLFW_KEY_UNKNOWN) {
            glfw_key_raw(*old_key, glfwGetKeyScancode(*old_key), GLFW_RELEASE, 0);
        }
        if (new_key != GLFW_KEY_UNKNOWN) {
            glfw_key_raw(new_key, glfwGetKeyScancode(new_key), GLFW_PRESS, 0);
        }
    }
    *old_key = new_key;
}
#define verify_controller_axis_value(value, threshold) \
    if ((value < (threshold) && value > 0) || (value > -(threshold) && value < 0)) { \
        value = 0; \
    }
#define CONTROLLER_MOVEMENT_AXIS_THRESHOLD 0.5f
static void glfw_controller_movement(float x, float y) {
    // Verify
    verify_controller_axis_value(x, CONTROLLER_MOVEMENT_AXIS_THRESHOLD);
    verify_controller_axis_value(y, CONTROLLER_MOVEMENT_AXIS_THRESHOLD);
    // Horizontal Movement
    if (x > 0) {
        release_and_press_key(&controller_horizontal_key, GLFW_KEY_D);
    } else if (x < 0) {
        release_and_press_key(&controller_horizontal_key, GLFW_KEY_A);
    } else {
        release_and_press_key(&controller_horizontal_key, GLFW_KEY_UNKNOWN);
    }
    // Vertical Movement
    if (y < 0) {
        release_and_press_key(&controller_vertical_key, GLFW_KEY_W);
    } else if (y > 0) {
        release_and_press_key(&controller_vertical_key, GLFW_KEY_S);
    } else {
        release_and_press_key(&controller_vertical_key, GLFW_KEY_UNKNOWN);
    }
}

// Get Time
#define NANOSECONDS_IN_SECOND 1000000000ll
static long long int get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long long int a = (long long int) ts.tv_nsec;
    long long int b = ((long long int) ts.tv_sec) * NANOSECONDS_IN_SECOND;
    return a + b;
}

// Controller Look Axis
#define CONTROLLER_LOOK_EVENT_PERIOD 50000000ll // 1/20 Seconds
#define CONTROLLER_LOOK_AXIS_THRESHOLD 0.2f
#define CONTROLLER_LOOK_AXIS_SENSITIVITY 70
static void glfw_controller_look(float x, float y) {
    // Current Time
    long long int current_time = get_time();
    // Last Time
    static long long int last_time = 0;
    static int is_last_time_set = 0;
    if (!is_last_time_set) {
        is_last_time_set = 1;
        last_time = current_time;
    }

    // Check If Period Has Passed
    if ((current_time - last_time) > CONTROLLER_LOOK_EVENT_PERIOD) {
        // Reset Last Time
        last_time = current_time;

        // Verify
        verify_controller_axis_value(x, CONTROLLER_LOOK_AXIS_THRESHOLD);
        verify_controller_axis_value(y, CONTROLLER_LOOK_AXIS_THRESHOLD);

        // Send Event
        SDL_Event event;
        event.type = SDL_MOUSEMOTION;
        event.motion.x = last_mouse_x;
        event.motion.y = last_mouse_y;
        event.motion.xrel = x * CONTROLLER_LOOK_AXIS_SENSITIVITY;
        event.motion.yrel = y * CONTROLLER_LOOK_AXIS_SENSITIVITY;
        SDL_PushEvent(&event);
    }
}

// Controller Place/Mine Triggers
#define CONTROLLER_TRIGGER_THRESHOLD 0
#define CONTROLLER_TRIGGER_COUNT 2
static void glfw_controller_trigger(int trigger, int action) {
    glfw_click_raw(trigger == GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER ? GLFW_MOUSE_BUTTON_LEFT : GLFW_MOUSE_BUTTON_RIGHT, action);
}

// Current Controller
static int current_controller = -1;

// Track Controller State
static void update_controller_state() {
    // Store Button/Trigger State
    static int controller_buttons[GLFW_GAMEPAD_BUTTON_LAST + 1];
    static int controller_triggers[CONTROLLER_TRIGGER_COUNT];

    // Get State
    GLFWgamepadstate state;
    int controller_enabled = cursor_grabbed && is_interactable;
    int controller_valid = controller_enabled && current_controller != -1 && glfwGetGamepadState(current_controller, &state);
    if (!controller_valid) {
        // Invalid Controller

        // Generate Blank State
        for (int i = GLFW_GAMEPAD_BUTTON_A; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
            state.buttons[i] = GLFW_RELEASE;
        }
        for (int i = GLFW_GAMEPAD_AXIS_LEFT_X; i <= GLFW_GAMEPAD_AXIS_LAST; i++) {
            int is_trigger = i == GLFW_GAMEPAD_AXIS_LEFT_TRIGGER || i == GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
            state.axes[i] = is_trigger ? -1 : 0;
        }
    }

    // Check Buttons
    for (int i = GLFW_GAMEPAD_BUTTON_A; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
        int old_state = controller_buttons[i];
        controller_buttons[i] = state.buttons[i];
        if (old_state != controller_buttons[i]) {
            // State Changed
            glfw_controller_button(i, controller_buttons[i]);
        }
    }

    // Handle Movement & Look
    glfw_controller_movement(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
    glfw_controller_look(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X], state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);

    // Check Triggers
    for (int i = 0; i < CONTROLLER_TRIGGER_COUNT; i++) {
        int old_state = controller_triggers[i];
        int trigger_id = i == 0 ? GLFW_GAMEPAD_AXIS_LEFT_TRIGGER : GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
        controller_triggers[i] = state.axes[trigger_id] < CONTROLLER_TRIGGER_THRESHOLD ? GLFW_RELEASE : GLFW_PRESS;
        if (old_state != controller_triggers[i]) {
            // State Changed
            glfw_controller_trigger(trigger_id, controller_triggers[i]);
        }
    }
}

// Pick Controller
static int joysticks[GLFW_JOYSTICK_LAST + 1];
static void pick_new_controller() {
    current_controller = -1;
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        if (joysticks[i] == 1) {
            current_controller = i;
            DEBUG("Using Controller: %s (%s)", glfwGetGamepadName(i), glfwGetJoystickName(i));
            break;
        }
    }
}
static void find_controllers() {
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        joysticks[i] = glfwJoystickIsGamepad(i);
    }
    pick_new_controller();
}
static void glfw_joystick(int jid, int event) {
    if (event == GLFW_CONNECTED && glfwJoystickIsGamepad(jid)) {
        joysticks[jid] = 1;
        pick_new_controller();
    } else if (event == GLFW_DISCONNECTED) {
        joysticks[jid] = 0;
        if (jid == current_controller) {
            DEBUG("Controller Disconnected");
            pick_new_controller();
        }
    }
}

// Release all keys/buttons when interaction is disabled and vice versa.
static void emit_events_after_is_interactable_change() {
    if (is_running) {
        for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; i++) {
            int state = glfwGetKey(glfw_window, i);
            if (state == GLFW_PRESS) {
                glfw_key_raw(i, glfwGetKeyScancode(i), is_interactable ? GLFW_PRESS : GLFW_RELEASE, 0);
            }
        }
        for (int i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; i++) {
            int state = glfwGetMouseButton(glfw_window, i);
            if (state == GLFW_PRESS) {
                glfw_click_raw(i, is_interactable ? GLFW_PRESS : GLFW_RELEASE);
            }
        }
    }
}

#endif

// Track If Raw Mouse Motion Is Enabled
static int raw_mouse_motion_enabled = 1;
void media_set_raw_mouse_motion_enabled(int enabled) {
    raw_mouse_motion_enabled = enabled;
#ifndef MCPI_HEADLESS_MODE
    if (is_running) {
        glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    }
#endif
    if (!raw_mouse_motion_enabled) {
        WARN("Raw mouse motion has been DISABLED, this IS NOT recommended, and should only ever be used on systems that don't support or have broken raw mouse motion.");
    }
}

// Disable V-Sync
static int disable_vsync = 0;
void media_disable_vsync() {
    disable_vsync = 1;
#ifndef MCPI_HEADLESS_MODE
    if (is_running) {
        glfwSwapInterval(0);
    }
#endif
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
typedef const unsigned char *(*glGetString_t)(unsigned int name);
void SDL_WM_SetCaption(const char *title, __attribute__((unused)) const char *icon) {
    // Don't Enable GLFW In Headless Mode
#ifndef MCPI_HEADLESS_MODE
    // Init GLFW
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

    // Setup Controller Support
    find_controllers();
    glfwSetJoystickCallback(glfw_joystick);

    // Make Window Context Current
    glfwMakeContextCurrent(glfw_window);

    // Debug
    glGetString_t glGetString = (glGetString_t) glfwGetProcAddress("glGetString");
    DEBUG("Using %s", (*glGetString)(GL_VERSION));

    // Init OpenAL
    _media_audio_init();
#else
    (void) title; // Mark As Used
#endif

    // Set State
    is_running = 1;

    // Update State
    update_cursor();
    if (disable_vsync) {
        media_disable_vsync();
    }

    // Always Cleanup Media Layer
    atexit(media_cleanup);
}

void media_swap_buffers() {
#ifndef MCPI_HEADLESS_MODE
    // Don't Swap Buffers In A Context-Less Window
    glfwSwapBuffers(glfw_window);
#endif
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
#else
void media_toggle_fullscreen() {
}
#endif

// Intercept SDL Events
void _media_handle_SDL_PollEvent() {
    // GLFW And Audio Are Disabled Disabled In Headless Mode
#ifndef MCPI_HEADLESS_MODE
    // Process GLFW Events
    glfwPollEvents();

    // Fix Joystick Detection While Running (Remove When glfw/glfw#2198 Is Merged)
    extern void _glfwDetectJoystickConnectionLinux(void);
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
        _glfwDetectJoystickConnectionLinux();
    }

    // Controller
    update_controller_state();

    // Close Window
    if (glfwWindowShouldClose(glfw_window)) {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
        glfwSetWindowShouldClose(glfw_window, GLFW_FALSE);
    }
#endif
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
#endif

        // Update State
        is_running = 0;
    }
}

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
#endif
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
#endif
    *width = DEFAULT_WIDTH;
    *height = DEFAULT_HEIGHT;
}
