#include <libreborn/libreborn.h>

// Do Nothing Function
static void do_nothing() {
    // NOP
}

// Patch bcm_host Calls
__attribute__((constructor)) static void patch_bcm_host_calls() {
    // Disable bcm_host Calls
    overwrite_call((void *) 0xdfec, (void *) do_nothing); // bcm_host_init
    overwrite_call((void *) 0x12418, (void *) do_nothing); // bcm_host_deinit
    overwrite_call((void *) 0x125a8, (void *) do_nothing); // graphics_get_display_size
    overwrite_call((void *) 0x125dc, (void *) do_nothing); // vc_dispmanx_display_open
    overwrite_call((void *) 0x125e8, (void *) do_nothing); // vc_dispmanx_update_start
    overwrite_call((void *) 0x12618, (void *) do_nothing); // vc_dispmanx_element_add
    overwrite_call((void *) 0x12624, (void *) do_nothing); // vc_dispmanx_update_submit_sync
}
