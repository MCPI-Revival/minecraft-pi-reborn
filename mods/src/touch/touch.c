#include <libreborn/libreborn.h>

#include "../feature/feature.h"
#include "../init/init.h"

#include <symbols/minecraft.h>

// Enable Touch GUI
static int32_t Minecraft_isTouchscreen_injection(__attribute__((unused)) unsigned char *minecraft) {
    return 1;
}

// Custom Cursor Rendering
// The Default Behavior For Touch GUI Is To Only Render The Cursor When The Mouse Is Clicking, This Fixes That
static void GameRenderer_render_injection(unsigned char *game_renderer, float param_1) {
    // Call Real Method
    (*GameRenderer_render)(game_renderer, param_1);

    // Render Cursor
    unsigned char *minecraft = *(unsigned char **) (game_renderer + GameRenderer_minecraft_property_offset);
    unsigned char *current_screen = *(unsigned char **) (minecraft + Minecraft_screen_property_offset);
    // Check If Cursor Should Render
    if (current_screen != NULL) {
        // Get X And Y
        float x = (*Mouse_getX)() * (*InvGuiScale);
        float y = (*Mouse_getY)() * (*InvGuiScale);
        // Render
        (*renderCursor)(x, y, minecraft);
    }
}

// Init
void init_touch() {
    int touch_gui = feature_has("Touch GUI", 0);
    if (touch_gui) {
        // Main UI
        overwrite((void *) Minecraft_isTouchscreen, (void *) Minecraft_isTouchscreen_injection);

        // Disable Normal Cursor Rendering
        unsigned char disable_cursor_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x4a6c0, disable_cursor_patch);
        // Add Custom Cursor Rendering
        overwrite_calls((void *) GameRenderer_render, (void *) GameRenderer_render_injection);

        // Force Correct Toolbar Size
        unsigned char toolbar_patch[4] = {0x01, 0x00, 0x50, 0xe3}; // "cmp r0, #0x1"
        patch((void *) 0x257b0, toolbar_patch);
    }

    // Show Block Outlines
    int block_outlines = feature_has("Show Block Outlines", 0);
    unsigned char outline_patch[4] = {block_outlines ? !touch_gui : touch_gui, 0x00, 0x50, 0xe3}; // "cmp r0, #0x1" or "cmp r0, #0x0"
    patch((void *) 0x4a210, outline_patch);
}
