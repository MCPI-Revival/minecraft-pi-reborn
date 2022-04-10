#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../feature/feature.h"
#include "input.h"

// Store Right-Click Status
static int is_right_click = 0;
void input_set_is_right_click(int val) {
    is_right_click = val;
}

// Enable Bow & Arrow Fix
static int fix_bow = 0;

// Handle Bow & Arrow
static void _handle_bow(unsigned char *minecraft) {
    if (fix_bow && !is_right_click) {
        // GameMode Is Offset From minecraft By 0x160
        // Player Is Offset From minecraft By 0x18c
        unsigned char *game_mode = *(unsigned char **) (minecraft + Minecraft_game_mode_property_offset);
        unsigned char *player = *(unsigned char **) (minecraft + Minecraft_player_property_offset);
        if (player != NULL && game_mode != NULL && (*Player_isUsingItem)(player)) {
            unsigned char *game_mode_vtable = *(unsigned char **) game_mode;
            GameMode_releaseUsingItem_t GameMode_releaseUsingItem = *(GameMode_releaseUsingItem_t *) (game_mode_vtable + GameMode_releaseUsingItem_vtable_offset);
            (*GameMode_releaseUsingItem)(game_mode, player);
        }
    }
}

// Init
void _init_bow() {
    // Enable Bow & Arrow Fix
    fix_bow = feature_has("Fix Bow & Arrow", server_disabled);
    input_run_on_tick(_handle_bow);
}
