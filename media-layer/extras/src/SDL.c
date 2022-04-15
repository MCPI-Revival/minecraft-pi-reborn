#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <X11/Xlib.h>
#include <sys/wait.h>

#include <libreborn/libreborn.h>
#include <media-layer/core.h>

// SDL Stub
void *SDL_SetVideoMode(__attribute__((unused)) int width, __attribute__((unused)) int height, __attribute__((unused)) int bpp, __attribute__((unused)) uint32_t flags) {
    // Return Value Is Only Used For A NULL-Check
    return (void *) 1;
}

static void x11_nop() {
    // NOP
}
int SDL_GetWMInfo(SDL_SysWMinfo *info) {
    // Return Fake Lock Functions Since XLib Isn't Directly Used
    SDL_SysWMinfo ret;
    ret.info.x11.lock_func = x11_nop;
    ret.info.x11.unlock_func = x11_nop;
    ret.info.x11.display = NULL;
    ret.info.x11.window = 0;
    ret.info.x11.wmwindow = ret.info.x11.window;
    *info = ret;
    return 1;
}

// Quit
__attribute__ ((noreturn)) void SDL_Quit() {
    // Cleanup Media Layer
    media_cleanup();

    // Wait For Children To Stop
    while (wait(NULL) > 0) {}

    // Exit
    INFO("Stopped");
    exit(EXIT_SUCCESS);
}
