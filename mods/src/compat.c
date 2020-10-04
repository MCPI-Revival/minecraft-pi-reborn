#define _GNU_SOURCE

#include <time.h>
#include <unistd.h>

#include <FreeImage.h>

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <X11/Xlib.h>

#include <libcore/libcore.h>

#include "extra.h"

static Display *x11_display;
static EGLDisplay egl_display;
static Window x11_window;
static Window x11_root_window;
static EGLConfig egl_config;
static int window_loaded = 0;
static EGLContext egl_context;
static EGLSurface egl_surface;
static SDL_Surface *sdl_surface;

HOOK(eglGetDisplay, EGLDisplay, (__attribute__((unused)) NativeDisplayType native_display)) {
    // Handled In ensure_x11_window()
    return 0;
}

// Get Reference To X Window
static void ensure_x11_window() {
    if (!window_loaded) {
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        SDL_GetWMInfo(&info);

        x11_display = info.info.x11.display;
        x11_window = info.info.x11.window;
        x11_root_window = RootWindow(x11_display, DefaultScreen(x11_display));
        ensure_eglGetDisplay();
        egl_display = (*real_eglGetDisplay)(x11_display);
        
        window_loaded = 1;
    }
}

// Handled In SDL_WM_SetCaption
HOOK(eglInitialize, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint *major, __attribute__((unused)) EGLint *minor)) {
    return EGL_TRUE;
}

// Handled In SDL_WM_SetCaption
HOOK(eglChooseConfig, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint const *attrib_list, __attribute__((unused)) EGLConfig *configs, __attribute__((unused)) EGLint config_size, __attribute__((unused)) EGLint *num_config)) {
    return EGL_TRUE;
}

// Handled In SDL_WM_SetCaption
HOOK(eglBindAPI, EGLBoolean, (__attribute__((unused)) EGLenum api)) {
    return EGL_TRUE;
}

// Handled In SDL_WM_SetCaption
HOOK(eglCreateContext, EGLContext, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) EGLContext share_context, __attribute__((unused)) EGLint const *attrib_list)) {
    return 0;
}

// Handled In SDL_WM_SetCaption
HOOK(eglCreateWindowSurface, EGLSurface, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) NativeWindowType native_window, __attribute__((unused)) EGLint const *attrib_list)) {
    return 0;
}

// Handled In SDL_WM_SetCaption
HOOK(eglMakeCurrent, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface draw, __attribute__((unused)) EGLSurface read, __attribute__((unused)) EGLContext context)) {
    return EGL_TRUE;
}

// Handled In SDL_Quit
HOOK(eglDestroySurface, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface)) {
    return EGL_TRUE;
}

// Handled In SDL_Quit
HOOK(eglDestroyContext, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLContext context)) {
    return EGL_TRUE;
}

// Handled In SDL_Quit
HOOK(eglTerminate, EGLBoolean, (__attribute__((unused)) EGLDisplay display)) {
    return EGL_TRUE;
}

// Handled In SDL_WM_SetCaption
HOOK(SDL_SetVideoMode, SDL_Surface *, (__attribute__((unused)) int width, __attribute__((unused)) int height, __attribute__((unused)) int bpp, __attribute__((unused)) uint32_t flags)) {
    // Return Value Is Only Used For A NULL-Check
    return (SDL_Surface *) 1;
}

// EGL Config
EGLint const set_attrib_list[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_DEPTH_SIZE, 16,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
};

#define WINDOW_VIDEO_FLAGS SDL_RESIZABLE
#define FULLSCREEN_VIDEO_FLAGS SDL_FULLSCREEN

#define BPP 32

// Init EGL
HOOK(SDL_WM_SetCaption, void, (const char *title, const char *icon)) {
    // Enable Unicode
    SDL_EnableUNICODE(SDL_ENABLE);

    FreeImage_Initialise(0);

    ensure_SDL_SetVideoMode();
    sdl_surface = (*real_SDL_SetVideoMode)(848, 480, BPP, WINDOW_VIDEO_FLAGS);
    
    ensure_SDL_WM_SetCaption();
    (*real_SDL_WM_SetCaption)(title, icon);

    ensure_x11_window();

    ensure_eglInitialize();
    (*real_eglInitialize)(egl_display, NULL, NULL);

    EGLint number_of_config;
    ensure_eglChooseConfig();
    (*real_eglChooseConfig)(egl_display, set_attrib_list, &egl_config, 1, &number_of_config);

    ensure_eglBindAPI();
    (*real_eglBindAPI)(EGL_OPENGL_ES_API);

    ensure_eglCreateContext();
    egl_context = (*real_eglCreateContext)(egl_display, egl_config, EGL_NO_CONTEXT, NULL);
    
    ensure_eglCreateWindowSurface();
    egl_surface = (*real_eglCreateWindowSurface)(egl_display, egl_config, x11_window, NULL);

    ensure_eglMakeCurrent();
    (*real_eglMakeCurrent)(egl_display, egl_surface, egl_surface, egl_context);
    
    eglSwapInterval(egl_display, 1);
}

HOOK(eglSwapBuffers, EGLBoolean, (__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface)) {
    ensure_eglSwapBuffers();
    EGLBoolean ret = (*real_eglSwapBuffers)(egl_display, egl_surface);

    return ret;
}

static void resize(int width, int height, int fullscreen) {
    uint32_t flags = fullscreen ? FULLSCREEN_VIDEO_FLAGS : WINDOW_VIDEO_FLAGS;

    ensure_SDL_SetVideoMode();
    sdl_surface = (*real_SDL_SetVideoMode)(width, height, BPP, flags);

    // OpenGL state modification for resizing
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glOrthox(0, width, 0, height, -1, 1);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
}

static int is_fullscreen = 0;

static int old_width = 0;
static int old_height = 0;

static void push_resize_event(int width, int height) {
    SDL_ResizeEvent resize;
    resize.type = SDL_VIDEORESIZE;
    resize.w = width;
    resize.h = height;

    SDL_Event event;
    event.type = SDL_VIDEORESIZE;
    event.resize = resize;

    SDL_PushEvent(&event);
}

static void toggle_fullscreen() {
    if (is_fullscreen) {
        push_resize_event(old_width, old_height);

        old_width = 0;
        old_height = 0;
    } else {
        old_width = sdl_surface->w;
        old_height = sdl_surface->h;

        Screen *screen = DefaultScreenOfDisplay(x11_display);
        push_resize_event(WidthOfScreen(screen), HeightOfScreen(screen));
    }
    is_fullscreen = !is_fullscreen;
}

// 4 (Year + 1 (Hyphen) + 2 (Month) + 1 (Hyphen) + 2 (Day) + 1 (Underscore) + 2 (Hour) + 1 (Period) + 2 (Minute) + 1 (Period) + 2 (Second) + 1 (Terminator)
#define TIME_SIZE 20

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

    int line_size = sdl_surface->w * 3;
    int size = sdl_surface->h * line_size;

    unsigned char pixels[size];
    glReadPixels(0, 0, sdl_surface->w, sdl_surface->h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

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

    FIBITMAP *image = FreeImage_ConvertFromRawBits(pixels, sdl_surface->w, sdl_surface->h, line_size, 24, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, 0);
    if (!FreeImage_Save(FIF_PNG, image, file, 0)) {
        fprintf(stderr, "Screenshot Failed: %s\n", file);
    } else {
        fprintf(stderr, "Screenshot Saved: %s\n", file);
    }
    FreeImage_Unload(image);

    free(file);
    free(screenshots);
}

HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    // Poll Events
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);

    // Resize EGL
    if (event != NULL && ret == 1) {
        int handled = 0;

        if (event->type == SDL_VIDEORESIZE) {
            resize(event->resize.w, event->resize.h, is_fullscreen);
            handled = 1;
        } else if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.sym == SDLK_F11) {
                toggle_fullscreen();
                handled = 1;
            } else if (event->key.keysym.sym == SDLK_F2) {
                screenshot();
                handled = 1;
            } else {
                key_press((char) event->key.keysym.unicode);
            }
        }

        if (handled) {
            // Event Was Handled
            return SDL_PollEvent(event);
        }
    }

    return ret;
}

// Terminate EGL
HOOK(SDL_Quit, void, ()) {
    ensure_SDL_Quit();
    (*real_SDL_Quit)();

    ensure_eglDestroyContext();
    (*real_eglDestroyContext)(egl_display, egl_context);
    ensure_eglDestroySurface();
    (*real_eglDestroySurface)(egl_display, egl_surface);
    ensure_eglTerminate();
    (*real_eglTerminate)(egl_display);
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

#include <stdlib.h>

// Use VirGL
__attribute__((constructor)) static void init() {
    setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1); 
    setenv("GALLIUM_DRIVER", "virpipe", 1);
}
