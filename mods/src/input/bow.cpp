#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include "input-internal.h"
#include <mods/input/input.h>

// Store Right-Click Status
static int is_right_click = 0;
void input_set_is_right_click(int val) {
    is_right_click = val;
}

// Enable Bow & Arrow Fix
static int fix_bow = 0;

// Handle Bow & Arrow
static void _handle_bow(Minecraft *minecraft) {
    if (fix_bow && !is_right_click) {
        GameMode *game_mode = minecraft->game_mode;
        LocalPlayer *player = minecraft->player;
        if (player != nullptr && game_mode != nullptr && player->isUsingItem()) {
            game_mode->vtable->releaseUsingItem(game_mode, (Player *) player);
        }
    }
}

// Init
void _init_bow() {
    // Enable Bow & Arrow Fix
    fix_bow = feature_has("Fix Bow & Arrow", server_disabled);
    input_run_on_tick(_handle_bow);
}
