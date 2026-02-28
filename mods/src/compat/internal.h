#pragma once

// Stubs
void _patch_egl_calls();
void _patch_x11_calls();
void _patch_bcm_host_calls();
void _patch_sdl_calls();

// Functionality
extern bool _compat_has_stopped;
void _init_compat_sdl();
