#include <libreborn/libreborn.h>

#include "../feature/feature.h"
#include "../init/init.h"

#include <symbols/minecraft.h>

// Enable Touch GUI
static int32_t Minecraft_isTouchscreen_injection(__attribute__((unused)) unsigned char *minecraft) {
    return 1;
}

// Init
void init_touch() {
    int touch_gui = feature_has("Touch GUI", 0);
    if (touch_gui) {
        // Main UI
        overwrite((void *) Minecraft_isTouchscreen, (void *) Minecraft_isTouchscreen_injection);

        // Force Correct Toolbar Size
        unsigned char toolbar_patch[4] = {0x01, 0x00, 0x50, 0xe3}; // "cmp r0, #0x1"
        patch((void *) 0x257b0, toolbar_patch);
    }

    // Show Block Outlines
    int block_outlines = feature_has("Show Block Outlines", 0);
    unsigned char outline_patch[4] = {block_outlines ? !touch_gui : touch_gui, 0x00, 0x50, 0xe3}; // "cmp r0, #0x1" or "cmp r0, #0x0"
    patch((void *) 0x4a210, outline_patch);
}
