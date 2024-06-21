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

// Handle Bow & Arrow
static void _handle_bow(Minecraft_tickInput_t original, Minecraft *minecraft) {
    if (!is_right_click) {
        GameMode *game_mode = minecraft->game_mode;
        LocalPlayer *player = minecraft->player;
        if (player != nullptr && game_mode != nullptr && player->isUsingItem()) {
            game_mode->releaseUsingItem((Player *) player);
        }
    }
    // Call Original Method
    original(minecraft);
}

// Init
void _init_bow() {
    // Enable Bow & Arrow Fix
    if (feature_has("Fix Bow & Arrow", server_disabled)) {
        overwrite_calls(Minecraft_tickInput, _handle_bow);
    }
}
