#include <EGL/egl.h>

#include "../compat/compat.h"

// EGL/SDL Is Replaced With GLFW

EGLDisplay eglGetDisplay(__attribute__((unused)) NativeDisplayType native_display) {
    return 0;
}
EGLBoolean eglInitialize(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint *major, __attribute__((unused)) EGLint *minor) {
    return EGL_TRUE;
}
EGLBoolean eglChooseConfig(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint const *attrib_list, __attribute__((unused)) EGLConfig *configs, __attribute__((unused)) EGLint config_size, __attribute__((unused)) EGLint *num_config) {
    return EGL_TRUE;
}
EGLBoolean eglBindAPI(__attribute__((unused)) EGLenum api) {
    return EGL_TRUE;
}
EGLContext eglCreateContext(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) EGLContext share_context, __attribute__((unused)) EGLint const *attrib_list) {
    return 0;
}
EGLSurface eglCreateWindowSurface(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) NativeWindowType native_window, __attribute__((unused)) EGLint const *attrib_list) {
    return 0;
}
EGLBoolean eglMakeCurrent(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface draw, __attribute__((unused)) EGLSurface read, __attribute__((unused)) EGLContext context) {
    return EGL_TRUE;
}
EGLBoolean eglDestroySurface(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface) {
    return EGL_TRUE;
}
EGLBoolean eglDestroyContext(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLContext context) {
    return EGL_TRUE;
}
EGLBoolean eglTerminate(__attribute__((unused)) EGLDisplay display) {
    return EGL_TRUE;
}

// Send Buffer Swap Request To GLFW
EGLBoolean eglSwapBuffers(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface) {
    compat_eglSwapBuffers();
    return EGL_TRUE;
}