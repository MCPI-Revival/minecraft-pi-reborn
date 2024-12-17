#include <SDL/SDL.h>

#include <media-layer/core.h>
#include <libreborn/patch.h>
#include "../compat-internal.h"

// SDL Stub
static void *SDL_SetVideoMode_injection(__attribute__((unused)) int width, __attribute__((unused)) int height, __attribute__((unused)) int bpp, __attribute__((unused)) uint32_t flags) {
    // Return Value Is Only Used For A NULL-Check
    return (void *) 1;
}

// Window Information
static void x11_nop() {
    // NOP
}
static int SDL_GetWMInfo_injection(SDL_SysWMinfo *info) {
    // Return Fake Lock Functions Since XLib Isn't Directly Used
    SDL_SysWMinfo ret;
    ret.info.x11.lock_func = x11_nop;
    ret.info.x11.unlock_func = x11_nop;
    ret.info.x11.display = nullptr;
    ret.info.x11.window = 0;
    ret.info.x11.wmwindow = ret.info.x11.window;
    *info = ret;
    return 1;
}

// Quit
static void SDL_Quit_injection() {
    // Cleanup Media Layer
    media_cleanup();
    // Exit
    INFO("Stopped");
}

// Patch SDL Calls
void _patch_sdl_calls() {
    // Disable SDL Calls
    overwrite_call_manual((void *) 0xe020, (void *) SDL_SetVideoMode_injection);
    overwrite_call_manual((void *) 0x13284, (void *) SDL_GetWMInfo_injection);
    overwrite_call_manual((void *) 0x12410, (void *) SDL_Quit_injection);
}
