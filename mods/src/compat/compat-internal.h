#pragma once

__attribute__((visibility("internal"))) void _patch_egl_calls();
__attribute__((visibility("internal"))) void _patch_x11_calls();
__attribute__((visibility("internal"))) void _patch_bcm_host_calls();
