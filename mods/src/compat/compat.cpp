#include <unistd.h>
#include <csignal>

#include <libreborn/util/io.h>
#include <libreborn/env/env.h>
#include <libreborn/patch.h>

#include <mods/compat/compat.h>
#include <mods/init/init.h>

#include "internal.h"

// Exit Handler
static void exit_handler(MCPI_UNUSED int data) {
    // Request Exit
    compat_request_exit();
}
void init_compat() {
    // Unlock Lock File
    const int lock_fd = std::stoi(require_env(_MCPI_LOCK_FD_ENV));
    unlock_file(lock_fd);

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

    // Stop Threads From Being Detached
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x14498, nop_patch);
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
