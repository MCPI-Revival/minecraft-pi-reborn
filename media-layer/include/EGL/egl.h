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

#ifdef __cplusplus
}
#endif
