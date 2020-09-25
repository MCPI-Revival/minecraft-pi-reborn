#define _GNU_SOURCE

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <X11/Xlib.h>

#include <libcore/libcore.h>

static Display *x11_display;
static EGLDisplay egl_display;
static Window x11_window;
static Window x11_root_window;
static EGLConfig egl_config;
static int window_loaded = 0;
static EGLContext egl_context;
static EGLSurface egl_surface;

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
HOOK(SDL_SetVideoMode, SDL_Surface *, (__attribute__((unused)) int width, __attribute__((unused)) int height, __attribute__((unused)) int bpp, __attribute__((unused)) Uint32 flags)) {
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

// Init EGL
HOOK(SDL_WM_SetCaption, void, (const char *title, const char *icon)) {
    ensure_SDL_SetVideoMode();
    (*real_SDL_SetVideoMode)(848, 480, 32, 16);
    
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
    ensure_x11_window();

    ensure_eglSwapBuffers();
    EGLBoolean ret = (*real_eglSwapBuffers)(egl_display, egl_surface);
    
    return ret;
}

HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);
    
    // Resize EGL
    if (event != NULL && event->type == SDL_VIDEORESIZE) {
        ensure_SDL_SetVideoMode();
        (*real_SDL_SetVideoMode)(event->resize.w, event->resize.h, 32, 16);

        // OpenGL state modification for resizing
        glViewport(0, 0, event->resize.w, event->resize.h);
        glMatrixMode(GL_PROJECTION);
        glOrthox(0, event->resize.w, 0, event->resize.h, -1, 1);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
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
