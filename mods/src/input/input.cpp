#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../feature/feature.h"
#include "../init/init.h"
#include "input.h"

// Run Functions On Input Tick
static std::vector<input_tick_function_t> &get_input_tick_functions() {
    static std::vector<input_tick_function_t> functions;
    return functions;
}
void input_run_on_tick(input_tick_function_t function) {
    get_input_tick_functions().push_back(function);
}

// Handle Input Fixes
static void Minecraft_tickInput_injection(unsigned char *minecraft) {
    // Call Original Method
    (*Minecraft_tickInput)(minecraft);

    // Run Input Tick Functions
    for (input_tick_function_t function : get_input_tick_functions()) {
        (*function)(minecraft);
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
    overwrite_calls((void *) Minecraft_tickInput, (void *) Minecraft_tickInput_injection);

    // Allow Attacking Mobs
    _init_attack();
}
