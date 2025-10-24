#include <media-layer/core.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include "internal.h"
#include <mods/input/input.h>

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

    // Allow Attacking Mobs
    _init_attack();

    // Disable Raw Mouse Motion
    if (feature_has("Disable Raw Mouse Motion (Not Recommended)", server_disabled)) {
        media_set_raw_mouse_motion_enabled(false);
    }

    // Extra Key Codes
    _init_keys();
}
