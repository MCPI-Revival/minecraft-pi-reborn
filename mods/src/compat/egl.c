#include <EGL/egl.h>

#include <libreborn/libreborn.h>
#include <media-layer/core.h>

// Functions That Have Their Return Values Used
static EGLSurface eglCreateWindowSurface_injection(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) NativeWindowType native_window, __attribute__((unused)) EGLint const *attrib_list) {
    return 0;
}
static EGLDisplay eglGetDisplay_injection(__attribute__((unused)) NativeDisplayType native_display) {
    return 0;
}
static EGLContext eglCreateContext_injection(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) EGLContext share_context, __attribute__((unused)) EGLint const *attrib_list) {
    return 0;
}
// Call media_swap_buffers()
static EGLBoolean eglSwapBuffers_injection(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface) {
    media_swap_buffers();
    return EGL_TRUE;
}

// Patch EGL Calls
__attribute__((constructor)) static void patch_egl_calls() {
    // Disable EGL Calls
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x11d3c, nop_patch); // eglTerminate
    patch((void *) 0x11dbc, nop_patch); // eglBindAPI
    overwrite_call((void *) 0x11e78, (void *) eglCreateWindowSurface_injection); // eglCreateWindowSurface
    patch((void *) 0x11db4, nop_patch); // eglChooseConfig
    patch((void *) 0x11d98, nop_patch); // eglInitialize
    patch((void *) 0x11d20, nop_patch); // eglMakeCurrent #1
    patch((void *) 0x11e90, nop_patch); // eglMakeCurrent #2
    overwrite_call((void *) 0x11d0c, (void *) eglSwapBuffers_injection); // eglSwapBuffers #1
    overwrite_call((void *) 0x14ce4, (void *) eglSwapBuffers_injection); // eglSwapBuffers #2
    overwrite_call((void *) 0x11d7c, (void *) eglGetDisplay_injection); // eglGetDisplay
    patch((void *) 0x11d2c, nop_patch); // eglDestroySurface #1
    patch((void *) 0x11d34, nop_patch); // eglDestroySurface #2
    overwrite_call((void *) 0x11dd0, (void *) eglCreateContext_injection); // eglCreateContext
}
