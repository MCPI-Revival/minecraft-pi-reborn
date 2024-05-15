#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <media-layer/core.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include "input-internal.h"
#include <mods/input/input.h>

// Run Functions On Input Tick
static std::vector<input_tick_function_t> &get_input_tick_functions() {
    static std::vector<input_tick_function_t> functions;
    return functions;
}
void input_run_on_tick(input_tick_function_t function) {
    get_input_tick_functions().push_back(function);
}

// Handle Input Fixes
static void Minecraft_tickInput_injection(Minecraft_tickInput_t original, Minecraft *minecraft) {
    // Call Original Method
    original(minecraft);

    // Run Input Tick Functions
    for (input_tick_function_t function : get_input_tick_functions()) {
        function(minecraft);
    }
}

// Init
void init_input() {
    // Miscellaneous
    _init_misc();

    // Toggleable Options
    _init_toggle();

    // Item Dropping
    _init_drop();

    // Enable Bow & Arrow Fix
    _init_bow();

    // Loop
    overwrite_calls(Minecraft_tickInput, Minecraft_tickInput_injection);

    // Allow Attacking Mobs
    _init_attack();

    // Allow Opening Crafting With Controller
    _init_crafting();

    // Disable Raw Mouse Motion
    if (feature_has("Disable Raw Mouse Motion (Not Recommended)", server_disabled)) {
        media_set_raw_mouse_motion_enabled(0);
    }
}
