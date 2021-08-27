#include <EGL/egl.h>

#include <libreborn/libreborn.h>

// EGL Is Replaced With GLFW

EGLDisplay eglGetDisplay(__attribute__((unused)) NativeDisplayType native_display) {
    IMPOSSIBLE();
}
EGLBoolean eglInitialize(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint *major, __attribute__((unused)) EGLint *minor) {
    IMPOSSIBLE();
}
EGLBoolean eglChooseConfig(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLint const *attrib_list, __attribute__((unused)) EGLConfig *configs, __attribute__((unused)) EGLint config_size, __attribute__((unused)) EGLint *num_config) {
    IMPOSSIBLE();
}
EGLBoolean eglBindAPI(__attribute__((unused)) EGLenum api) {
    IMPOSSIBLE();
}
EGLContext eglCreateContext(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) EGLContext share_context, __attribute__((unused)) EGLint const *attrib_list) {
    IMPOSSIBLE();
}
EGLSurface eglCreateWindowSurface(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLConfig config, __attribute__((unused)) NativeWindowType native_window, __attribute__((unused)) EGLint const *attrib_list) {
    IMPOSSIBLE();
}
EGLBoolean eglMakeCurrent(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface draw, __attribute__((unused)) EGLSurface read, __attribute__((unused)) EGLContext context) {
    IMPOSSIBLE();
}
EGLBoolean eglDestroySurface(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface) {
    IMPOSSIBLE();
}
EGLBoolean eglDestroyContext(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLContext context) {
    IMPOSSIBLE();
}
EGLBoolean eglTerminate(__attribute__((unused)) EGLDisplay display) {
    IMPOSSIBLE();
}
EGLBoolean eglSwapBuffers(__attribute__((unused)) EGLDisplay display, __attribute__((unused)) EGLSurface surface) {
    IMPOSSIBLE();
}
