#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include "input-internal.h"
#include <mods/input/input.h>

// Store Left Click (0 = Not Pressed, 1 = Pressed)
static int is_left_click = 0;
void input_set_is_left_click(int val) {
    if ((is_left_click == 0 && val == 1) || (is_left_click != 0 && val == 0)) {
        is_left_click = val;
    }
}

// Add Attacking To MouseBuildInput
static int32_t MouseBuildInput_tickBuild_injection(unsigned char *mouse_build_input, unsigned char *local_player, uint32_t *build_action_intention_return) {
    // Call Original Method
    int32_t ret = (*MouseBuildInput_tickBuild)(mouse_build_input, local_player, build_action_intention_return);

    // Use Attack/Place BuildActionIntention If No Other Valid BuildActionIntention Was Selected And This Was Not A Repeated Left Click
    if (ret != 0 && is_left_click == 1 && *build_action_intention_return == 0xa) {
        // Get Target HitResult
        unsigned char *minecraft = *(unsigned char **) (local_player + LocalPlayer_minecraft_property_offset);
        HitResult *hit_result = (HitResult *) (minecraft + Minecraft_hit_result_property_offset);
        int32_t hit_result_type = hit_result->type;
        // Check if The Target Is An Entity Using HitResult
        if (hit_result_type == 1) {
            // Change BuildActionIntention To Attack/Place Mode (Place Will Not Happen Because The HitResult Is An Entity)
            *build_action_intention_return = 0x8;
        }
    }

    // Return
    return ret;
}

// Init
void _init_attack() {
    // Allow Attacking Mobs
    if (feature_has("Fix Attacking", server_disabled)) {
        patch_address(MouseBuildInput_tickBuild_vtable_addr, (void *) MouseBuildInput_tickBuild_injection);
    }
}
