#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <X11/Xlib.h>

#define EGL_TRUE 1

typedef unsigned int EGLenum;
typedef unsigned int EGLBoolean;
typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;
typedef int32_t EGLint;

typedef void *EGLNativeDisplayType;
typedef XID EGLNativeWindowType;
typedef EGLNativeWindowType NativeWindowType;
typedef EGLNativeDisplayType NativeDisplayType;

EGLDisplay eglGetDisplay(NativeDisplayType native_display);
EGLBoolean eglInitialize(EGLDisplay display, EGLint *major, EGLint *minor);
EGLBoolean eglChooseConfig(EGLDisplay display, EGLint const *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLBoolean eglBindAPI(EGLenum api);
EGLContext eglCreateContext(EGLDisplay display, EGLConfig config, EGLContext share_context, EGLint const *attrib_list);
EGLSurface eglCreateWindowSurface(EGLDisplay display, EGLConfig config, NativeWindowType native_window, EGLint const *attrib_list);
EGLBoolean eglMakeCurrent(EGLDisplay display, EGLSurface draw, EGLSurface read, EGLContext context);
EGLBoolean eglDestroySurface(EGLDisplay display, EGLSurface surface);
EGLBoolean eglDestroyContext(EGLDisplay display, EGLContext context);
EGLBoolean eglTerminate(EGLDisplay display);
EGLBoolean eglSwapBuffers(EGLDisplay display, EGLSurface surface);

#ifdef __cplusplus
}
#endif
