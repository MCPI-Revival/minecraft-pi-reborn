#define _GNU_SOURCE

#include <time.h>
#include <unistd.h>

#include <FreeImage.h>

#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_INCLUDE_ES1
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <X11/extensions/Xfixes.h>

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <X11/Xlib.h>

#include <libcore/libcore.h>

#include "extra.h"

static GLFWwindow *glfw_window;
static Display *x11_display;
static Window x11_window;
static Window x11_root_window;
static int window_loaded = 0;

static int is_server = 0;

// Get Reference To X Window
static void store_x11_window() {
    x11_display = glfwGetX11Display();
    x11_window = glfwGetX11Window(glfw_window);
    x11_root_window = RootWindow(x11_display, DefaultScreen(x11_display));

    window_loaded = 1;
}

// Handle GLFW Error
static void glfw_error(__attribute__((unused)) int error, const char *description) {
    ERR("GLFW Error: %s", description);
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
        // Unknown
        default:
            return SDLK_UNKNOWN;
    }
}

// Pass Key Presses To SDL
static void glfw_key(__attribute__((unused)) GLFWwindow *window, int key, int scancode, int action, __attribute__((unused)) int mods) {
    SDL_Event event;
    int up = action == GLFW_RELEASE;
    event.type = up ? SDL_KEYUP : SDL_KEYDOWN;
    event.key.state = up ? SDL_RELEASED : SDL_PRESSED;
    event.key.keysym.scancode = scancode;
    event.key.keysym.mod = KMOD_NONE;
    event.key.keysym.sym = glfw_key_to_sdl_key(key);
    SDL_PushEvent(&event);
    if (key == GLFW_KEY_BACKSPACE && !up) {
        extra_key_press((char) '\b');
    }
}

// Pass Text To Minecraft
static void glfw_char(__attribute__((unused)) GLFWwindow *window, unsigned int codepoint) {
    extra_key_press((char) codepoint);
}

static double last_mouse_x = 0;
static double last_mouse_y = 0;

// Pass Mouse Movement To SDL
static void glfw_motion(__attribute__((unused)) GLFWwindow *window, double xpos, double ypos) {
    SDL_Event event;
    event.type = SDL_MOUSEMOTION;
    event.motion.x = xpos;
    event.motion.y = ypos;
    event.motion.xrel = (xpos - last_mouse_x);
    event.motion.yrel = (ypos - last_mouse_y);
    last_mouse_x = xpos;
    last_mouse_y = ypos;
    SDL_PushEvent(&event);
}

// Create And Push SDL Mouse Click Event
static void click(int button, int up) {
    SDL_Event event;
    event.type = up ? SDL_MOUSEBUTTONUP : SDL_MOUSEBUTTONDOWN;
    event.button.x = last_mouse_x;
    event.button.y = last_mouse_y;
    event.button.state = up ? SDL_RELEASED : SDL_PRESSED;
    event.button.button = button;
    SDL_PushEvent(&event);

    if (button == SDL_BUTTON_RIGHT) {
        extra_set_is_right_click(!up);
    }
}

// Pass Mouse Click To SDL
static void glfw_click(__attribute__((unused)) GLFWwindow *window, int button, int action, __attribute__((unused)) int mods) {
    int up = action == GLFW_RELEASE;
    int sdl_button = button == GLFW_MOUSE_BUTTON_RIGHT ? SDL_BUTTON_RIGHT : (button == GLFW_MOUSE_BUTTON_LEFT ? SDL_BUTTON_LEFT : SDL_BUTTON_MIDDLE);
    click(sdl_button, up);
}

// Pass Mouse Scroll To SDL
static void glfw_scroll(__attribute__((unused)) GLFWwindow *window, __attribute__((unused)) double xoffset, double yoffset) {
    if (yoffset != 0) {
        int sdl_button = yoffset > 0 ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
        click(sdl_button, 0);
        click(sdl_button, 1);
    }
}

// Init GLFW
HOOK(SDL_WM_SetCaption, void, (const char *title, __attribute__((unused)) const char *icon)) {
    FreeImage_Initialise(0);

    glfwSetErrorCallback(glfw_error);

    if (!glfwInit()) {
        fprintf(stderr, "Unable To Initialize GLFW\n");
        exit(1);
    }

    if (is_server) {
        // Don't Show Window In Server Mode
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else {
        // Create OpenGL ES 1.1 Context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    glfw_window = glfwCreateWindow(840, 480, title, NULL, NULL);
    if (!glfw_window) {
        fprintf(stderr, "Unable To Create GLFW Window\n");
        exit(1);
    }

    if (!is_server) {
        // Don't Process Events In Server Mode
        glfwSetKeyCallback(glfw_window, glfw_key);
        glfwSetCharCallback(glfw_window, glfw_char);
        glfwSetCursorPosCallback(glfw_window, glfw_motion);
        glfwSetMouseButtonCallback(glfw_window, glfw_click);
        glfwSetScrollCallback(glfw_window, glfw_scroll);
    }

    store_x11_window();

    if (!is_server) {
        glfwMakeContextCurrent(glfw_window);
    }
}

HOOK(eglSwapBuffers, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface)) {
    if (!is_server) {
        // Don't Swap Buffers In A Context-Less Window
        glfwSwapBuffers(glfw_window);
    }

    return EGL_TRUE;
}

static int is_fullscreen = 0;

static int old_width = -1;
static int old_height = -1;

static int old_x = -1;
static int old_y = -1;

// Toggle Fullscreen
static void toggle_fullscreen() {
    if (is_fullscreen) {
        glfwSetWindowMonitor(glfw_window, NULL, old_x, old_y, old_width, old_height, GLFW_DONT_CARE);

        old_width = -1;
        old_height = -1;
        old_x = -1;
        old_y = -1;
    } else {
        glfwGetWindowSize(glfw_window, &old_width, &old_height);
        glfwGetWindowPos(glfw_window, &old_x, &old_y);
        Screen *screen = DefaultScreenOfDisplay(x11_display);

        glfwSetWindowMonitor(glfw_window, glfwGetPrimaryMonitor(), 0, 0, WidthOfScreen(screen), HeightOfScreen(screen), GLFW_DONT_CARE);
    }
    is_fullscreen = !is_fullscreen;
}

// 4 (Year + 1 (Hyphen) + 2 (Month) + 1 (Hyphen) + 2 (Day) + 1 (Underscore) + 2 (Hour) + 1 (Period) + 2 (Minute) + 1 (Period) + 2 (Second) + 1 (Terminator)
#define TIME_SIZE 20

// Take Screenshot
static void screenshot() {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time[TIME_SIZE];
    strftime(time, TIME_SIZE, "%Y-%m-%d_%H.%M.%S", timeinfo);

    char *screenshots = NULL;
    asprintf(&screenshots, "%s/.minecraft/screenshots", getenv("HOME"));

    int num = 1;
    char *file = NULL;
    asprintf(&file, "%s/%s.png", screenshots, time);
    while (access(file, F_OK) != -1) {
        asprintf(&file, "%s/%s-%i.png", screenshots, time, num);
        num++;
    }

    int width;
    int height;
    glfwGetWindowSize(glfw_window, &width, &height);

    int line_size = width * 3;
    int size = height * line_size;

    unsigned char pixels[size];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    // Swap Red And Blue
    for (int i = 0; i < (size / 3); i++) {
        int pixel = i * 3;
        int red = pixels[pixel];
        int blue = pixels[pixel + 2];
        pixels[pixel] = blue;
        pixels[pixel + 2] = red;
    }
#endif

    FIBITMAP *image = FreeImage_ConvertFromRawBits(pixels, width, height, line_size, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0);
    if (!FreeImage_Save(FIF_PNG, image, file, 0)) {
        fprintf(stderr, "Screenshot Failed: %s\n", file);
    } else {
        fprintf(stderr, "Screenshot Saved: %s\n", file);
    }
    FreeImage_Unload(image);

    free(file);
    free(screenshots);
}

// Intercept SDL Events
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    // Process GLFW Events
    glfwPollEvents();

    // Close Window
    if (glfwWindowShouldClose(glfw_window)) {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
        glfwSetWindowShouldClose(glfw_window, GLFW_FALSE);
    }

    // Poll Events
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);

    // Handle Events
    if (ret == 1 && event != NULL) {
        int handled = 0;

        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.sym == SDLK_F11) {
                toggle_fullscreen();
                handled = 1;
            } else if (event->key.keysym.sym == SDLK_F2) {
                screenshot();
                handled = 1;
            }
        }

        if (handled) {
            // Event Was Handled
            return SDL_PollEvent(event);
        }
    }

    return ret;
}

// Terminate GLFW
HOOK(SDL_Quit, void, ()) {
    ensure_SDL_Quit();
    (*real_SDL_Quit)();

    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

static SDL_GrabMode fake_grab_mode = SDL_GRAB_OFF;

// Fix SDL Cursor Visibility/Grabbing
HOOK(SDL_WM_GrabInput, SDL_GrabMode, (SDL_GrabMode mode)) {
    if (is_server) {
        // Don't Grab Input In Server/Headless Mode
        if (mode != SDL_GRAB_QUERY) {
            fake_grab_mode = mode;
        }
        return fake_grab_mode;
    } else {
        if (mode != SDL_GRAB_QUERY && mode != SDL_WM_GrabInput(SDL_GRAB_QUERY)) {
            glfwSetInputMode(glfw_window, GLFW_CURSOR, mode == SDL_GRAB_OFF ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            glfwSetInputMode(glfw_window, GLFW_RAW_MOUSE_MOTION, mode == SDL_GRAB_OFF ? GLFW_FALSE : GLFW_TRUE);

            // GLFW Cursor Hiding is Broken
            if (window_loaded) {
                if (mode == SDL_GRAB_OFF) {
                    XFixesShowCursor(x11_display, x11_window);
                } else {
                    XFixesHideCursor(x11_display, x11_window);
                }
                XFlush(x11_display);
            }
        }
        return mode == SDL_GRAB_QUERY ? (glfwGetInputMode(glfw_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL ? SDL_GRAB_OFF : SDL_GRAB_ON) : mode;
    }
}

// Stub SDL Cursor Visibility
HOOK(SDL_ShowCursor, int, (int toggle)) {
    if (is_server) {
        return toggle == SDL_QUERY ? (fake_grab_mode == SDL_GRAB_OFF ? SDL_ENABLE : SDL_DISABLE) : toggle;
    } else {
        return toggle == SDL_QUERY ? (glfwGetInputMode(glfw_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL ? SDL_ENABLE : SDL_DISABLE) : toggle;
    }
}

// SDL Stub
HOOK(SDL_SetVideoMode, SDL_Surface *, (__attribute__((unused)) int width, __attribute__((unused)) int height, __attribute__((unused)) int bpp, __attribute__((unused)) uint32_t flags)) {
    // Return Value Is Only Used For A NULL-Check
    return (SDL_Surface *) 1;
}

HOOK(XTranslateCoordinates, int, (Display *display, Window src_w, Window dest_w, int src_x, int src_y, int *dest_x_return, int *dest_y_return, Window *child_return)) {
    ensure_XTranslateCoordinates();
    if (window_loaded) {
        return (*real_XTranslateCoordinates)(x11_display, x11_window, x11_root_window, src_x, src_y, dest_x_return, dest_y_return, child_return);
    } else {
        return (*real_XTranslateCoordinates)(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, child_return);
    }
}

HOOK(XGetWindowAttributes, int, (Display *display, Window w, XWindowAttributes *window_attributes_return)) {
    ensure_XGetWindowAttributes();
    if (window_loaded) {
        return (*real_XGetWindowAttributes)(x11_display, x11_window, window_attributes_return);
    } else {
        return (*real_XGetWindowAttributes)(display, w, window_attributes_return);
    }
}

// EGL Stubs

HOOK(eglGetDisplay, EGLDisplay, (__attribute__((unused)) NativeDisplayType native_display)) {
    return 0;
}
HOOK(eglInitialize, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint *major, __attribute__((unused)) EGLint *minor)) {
    return EGL_TRUE;
}
HOOK(eglChooseConfig, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint const *attrib_list, __attribute__((unused)) EGLConfig *configs, __attribute__((unused)) EGLint config_size, __attribute__((unused)) EGLint *num_config)) {
    return EGL_TRUE;
}
HOOK(eglBindAPI, EGLBoolean, (__attribute__((unused)) EGLenum api)) {
    return EGL_TRUE;
}
HOOK(eglCreateContext, EGLContext, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) EGLContext share_context, __attribute__((unused)) EGLint const *attrib_list)) {
    return 0;
}
HOOK(eglCreateWindowSurface, EGLSurface, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) NativeWindowType native_window, __attribute__((unused)) EGLint const *attrib_list)) {
    return 0;
}
HOOK(eglMakeCurrent, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface draw, __attribute__((unused)) EGLSurface read, __attribute__((unused)) EGLContext context)) {
    return EGL_TRUE;
}
HOOK(eglDestroySurface, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface)) {
    return EGL_TRUE;
}
HOOK(eglDestroyContext, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLContext context)) {
    return EGL_TRUE;
}
HOOK(eglTerminate, EGLBoolean, (__attribute__((unused)) EGLDisplay display)) {
    return EGL_TRUE;
}

#include <stdlib.h>

// Use VirGL
__attribute__((constructor)) static void init() {
    int mode = extra_get_mode();
    if (mode != 1) {
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
    }
    if (mode == 0) {
        setenv("GALLIUM_DRIVER", "virpipe", 1);
    }
    is_server = mode == 2;
}
