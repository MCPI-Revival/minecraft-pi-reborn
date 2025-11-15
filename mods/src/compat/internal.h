#pragma once

// Stubs
MCPI_INTERNAL void _patch_egl_calls();
MCPI_INTERNAL void _patch_x11_calls();
MCPI_INTERNAL void _patch_bcm_host_calls();
MCPI_INTERNAL void _patch_sdl_calls();

// Functionality
MCPI_INTERNAL extern bool _compat_has_stopped;
MCPI_INTERNAL void _init_compat_sdl();
