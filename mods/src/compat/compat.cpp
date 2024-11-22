#include <unistd.h>
#include <csignal>

#include <mods/compat/compat.h>
#include <mods/init/init.h>

#include "compat-internal.h"

// Exit Handler
static void exit_handler(__attribute__((unused)) int data) {
    // Request Exit
    compat_request_exit();
}
void init_compat() {
    // SDL
    _init_compat_sdl();
    // Install Signal Handlers
    struct sigaction act_sigint = {};
    act_sigint.sa_flags = SA_RESTART;
    act_sigint.sa_handler = &exit_handler;
    sigaction(SIGINT, &act_sigint, nullptr);
    struct sigaction act_sigterm = {};
    act_sigterm.sa_flags = SA_RESTART;
    act_sigterm.sa_handler = &exit_handler;
    sigaction(SIGTERM, &act_sigterm, nullptr);
    // Patches
    _patch_egl_calls();
    _patch_x11_calls();
    _patch_bcm_host_calls();
    _patch_sdl_calls();
}

// Store Exit Requests
static int exit_requested = 0;
int compat_check_exit_requested() {
    if (exit_requested) {
        exit_requested = 0;
        return 1;
    } else {
        return 0;
    }
}
void compat_request_exit() {
    // Request
    exit_requested = 1;
}
