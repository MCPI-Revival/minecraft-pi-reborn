/*
 * This is only loaded when no other EGL library is present on the system to silence linker errors.
 *
 * All EGL calls are manually patched out in mods/src/compat/egl.c.
 * Unlike with bcm_host, EGL can't just be always stubbed out with LD_PRELOAD, because in some situations GLFW has it as a dependency.
 *
 * So normally, MCPI just loads the real EGL and never uses it (because MCPI's EGL calls were patched out).
 * However, since the real EGL is still loaded, GLFW can use it if it needs to.
 *
 * This stub library is just in case the system has no EGL library present.
 */

#include <EGL/egl.h>

#include <libreborn/libreborn.h>

#define IMPOSSIBLE() ERR("(%s:%i) This Should Never Be Called", __FILE__, __LINE__)

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
