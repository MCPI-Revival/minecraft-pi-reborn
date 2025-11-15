#include <EGL/egl.h>
#include <media-layer/core.h>

#include <libreborn/patch.h>
#include <symbols/minecraft.h>

#include <mods/misc/misc.h>

#include "../internal.h"

// Functions That Have Their Return Values Used
static EGLSurface eglCreateWindowSurface_injection(MCPI_UNUSED EGLDisplay display, MCPI_UNUSED EGLConfig config, MCPI_UNUSED NativeWindowType native_window, MCPI_UNUSED EGLint const *attrib_list) {
    return nullptr;
}
static EGLDisplay eglGetDisplay_injection(MCPI_UNUSED NativeDisplayType native_display) {
    return nullptr;
}
static EGLContext eglCreateContext_injection(MCPI_UNUSED EGLDisplay display, MCPI_UNUSED EGLConfig config, MCPI_UNUSED EGLContext share_context, MCPI_UNUSED EGLint const *attrib_list) {
    return nullptr;
}

// Call media_swap_buffers()
static EGLBoolean eglSwapBuffers_injection(MCPI_UNUSED EGLDisplay display, MCPI_UNUSED EGLSurface surface) {
    media_swap_buffers();
    return EGL_TRUE;
}

// Proper Cleanup
static Minecraft *minecraft = nullptr;
static void Minecraft_init_injection(Minecraft *self) {
    minecraft = self;
}
static EGLBoolean eglTerminate_injection(MCPI_UNUSED EGLDisplay display) {
    if (_compat_has_stopped && minecraft && !minecraft->generating_level) {
        minecraft->destructor_deleting();
    }
    return EGL_TRUE;
}

// Patch EGL Calls
void _patch_egl_calls() {
    // Disable EGL Calls
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x12580, nop_patch); // eglBindAPI
    overwrite_call_manual((void *) 0x12638, (void *) eglCreateWindowSurface_injection); // eglCreateWindowSurface
    patch((void *) 0x12578, nop_patch); // eglChooseConfig
    patch((void *) 0x1255c, nop_patch); // eglInitialize
    patch((void *) 0x124f0, nop_patch); // eglMakeCurrent #1
    patch((void *) 0x12654, nop_patch); // eglMakeCurrent #2
    overwrite_call_manual((void *) 0x124dc, (void *) eglSwapBuffers_injection); // eglSwapBuffers #1
    overwrite_call_manual((void *) 0x14b6c, (void *) eglSwapBuffers_injection); // eglSwapBuffers #2
    overwrite_call_manual((void *) 0x1254c, (void *) eglGetDisplay_injection); // eglGetDisplay
    patch((void *) 0x124fc, nop_patch); // eglDestroySurface #1
    patch((void *) 0x12504, nop_patch); // eglDestroySurface #2
    overwrite_call_manual((void *) 0x12594, (void *) eglCreateContext_injection); // eglCreateContext
    // Proper Cleanup
    misc_run_on_init(Minecraft_init_injection);
    overwrite_call_manual((void *) 0x1250c, (void *) eglTerminate_injection);
}
