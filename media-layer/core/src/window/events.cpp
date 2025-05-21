#include "media.h"

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
        // Debug
        case GLFW_KEY_F3:
            return SDLK_F3;
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
    media_SDL_PushEvent(&event1);
    // Allow MCPI To Access Original GLFW Keycode
    SDL_Event event2;
    event2.type = SDL_USEREVENT;
    event2.user.code = USER_EVENT_REAL_KEY;
    event2.user.data1 = event1.key.state;
    event2.user.data2 = key;
    media_SDL_PushEvent(&event2);
}
static void glfw_key(MCPI_UNUSED GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    if (is_interactable) {
        glfw_key_raw(key, scancode, action, mods);
    }
}

// Pass Text To Minecraft
static void character_event(unsigned char c) {
    if (!is_interactable) {
        return;
    }
    // SDL_UserEvent Is Never Used In MCPI, So It Is Repurposed For Character Events
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = USER_EVENT_CHARACTER;
    event.user.data1 = (int) c;
    media_SDL_PushEvent(&event);
}
static void glfw_char(MCPI_UNUSED GLFWwindow *window, const unsigned int codepoint) {
    character_event(utf32_to_cp437(codepoint));
}

// Convert Screen Coordinates To Pixels
static void convert_to_pixels(GLFWwindow *window, double *xpos, double *ypos) {
    // Skip If Cursor Is Grabbed
    if (media_SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON && raw_mouse_motion_enabled) {
        return;
    }
    // Get Scale
    float x_scale = 1;
    float y_scale = 1;
    get_glfw_scale(window, &x_scale, &y_scale);
    // Multiply
    *xpos *= x_scale;
    *ypos *= y_scale;
}

// Last Mouse Location
static double last_mouse_x = 0;
static double last_mouse_y = 0;

// Pass Mouse Movement To SDL
void _media_glfw_motion(MCPI_UNUSED GLFWwindow *window, double xpos, double ypos) {
    convert_to_pixels(window, &xpos, &ypos);
    if (is_interactable) {
        SDL_Event event;
        event.type = SDL_MOUSEMOTION;
        event.motion.x = uint16_t(xpos);
        event.motion.y = uint16_t(ypos);
        event.motion.xrel = !ignore_relative_motion ? (xpos - last_mouse_x) : 0;
        event.motion.yrel = !ignore_relative_motion ? (ypos - last_mouse_y) : 0;
        media_SDL_PushEvent(&event);
    }
    ignore_relative_motion = false;
    last_mouse_x = xpos;
    last_mouse_y = ypos;
}

// Create And Push SDL Mouse Click Event
static void click_event(int button, bool up) {
    SDL_Event event;
    event.type = up ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN;
    event.button.x = uint16_t(last_mouse_x);
    event.button.y = uint16_t(last_mouse_y);
    event.button.state = up ? SDL_RELEASED : SDL_PRESSED;
    event.button.button = button;
    media_SDL_PushEvent(&event);
}

// Pass Mouse Click To SDL
static void glfw_click_raw(const int button, const int action) {
    const bool up = action == GLFW_RELEASE;
    const int sdl_button = button == GLFW_MOUSE_BUTTON_RIGHT ? SDL_BUTTON_RIGHT : (button == GLFW_MOUSE_BUTTON_LEFT ? SDL_BUTTON_LEFT : SDL_BUTTON_MIDDLE);
    click_event(sdl_button, up);
}
static void glfw_click(MCPI_UNUSED GLFWwindow *window, const int button, const int action, MCPI_UNUSED int mods) {
    if (is_interactable) {
        glfw_click_raw(button, action);
    }
}

// Pass Mouse Scroll To SDL
static void glfw_scroll(MCPI_UNUSED GLFWwindow *window, MCPI_UNUSED double xoffset, double yoffset) {
    if (is_interactable && yoffset != 0) {
        const int sdl_button = yoffset > 0 ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
        click_event(sdl_button, false);
        click_event(sdl_button, true);
    }
}

// Intercept SDL Events
void _media_handle_media_SDL_PollEvent() {
    if (glfw_window) {
        // Process GLFW Events
        glfwPollEvents();
        // Close Window
        if (glfwWindowShouldClose(glfw_window)) {
            SDL_Event event;
            event.type = SDL_QUIT;
            media_SDL_PushEvent(&event);
            glfwSetWindowShouldClose(glfw_window, GLFW_FALSE);
        }
    }
}

// Register Event Listeners
void _media_register_event_listeners() {
    glfwSetKeyCallback(glfw_window, glfw_key);
    glfwSetCharCallback(glfw_window, glfw_char);
    glfwSetCursorPosCallback(glfw_window, _media_glfw_motion);
    glfwSetMouseButtonCallback(glfw_window, glfw_click);
    glfwSetScrollCallback(glfw_window, glfw_scroll);
}