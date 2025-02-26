#pragma once

// Stubs
__attribute__((visibility("internal"))) void _patch_egl_calls();
__attribute__((visibility("internal"))) void _patch_x11_calls();
__attribute__((visibility("internal"))) void _patch_bcm_host_calls();
__attribute__((visibility("internal"))) void _patch_sdl_calls();

// Functionality
__attribute__((visibility("internal"))) void _init_compat_sdl();
