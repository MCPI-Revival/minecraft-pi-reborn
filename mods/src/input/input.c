#include <libreborn/libreborn.h>

#include "../feature/feature.h"
#include "input.h"
#include "../init/init.h"
#include "../chat/chat.h"

#include <libreborn/minecraft.h>

// Store Right-Click Status
static int is_right_click = 0;
void input_set_is_right_click(int val) {
    is_right_click = val;
}

// Enable Bow & Arrow Fix
static int fix_bow = 0;

// Store Function Input
static int hide_gui_toggle = 0;
void input_hide_gui() {
    hide_gui_toggle++;
}
static int third_person_toggle = 0;
void input_third_person() {
    third_person_toggle++;
}

// Set mouse Grab State
static int mouse_grab_state = 0;
void input_set_mouse_grab_state(int state) {
    mouse_grab_state = state;
}

// Handle Input Fixes
static void Minecraft_tickInput_injection(unsigned char *minecraft) {
    // Call Original Method
    (*Minecraft_tickInput)(minecraft);

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

    // Clear Unused Sign Input
    input_clear_input();

    // Handle Functions
    unsigned char *options = minecraft + Minecraft_options_property_offset;
    if (hide_gui_toggle % 2 != 0) {
        // Toggle Hide GUI
        *(options + Options_hide_gui_property_offset) = *(options + Options_hide_gui_property_offset) ^ 1;
    }
    hide_gui_toggle = 0;
    if (third_person_toggle % 2 != 0) {
        // Toggle Third Person
        *(options + Options_third_person_property_offset) = *(options + Options_third_person_property_offset) ^ 1;
    }
    third_person_toggle = 0;

    // Send Queued Chat Message
    chat_send_messages(minecraft);

    // Set Mouse Grab State
    if (mouse_grab_state == -1) {
        // Grab
        (*Minecraft_grabMouse)(minecraft);
    } else if (mouse_grab_state == 1) {
        // Un-Grab
        (*Minecraft_releaseMouse)(minecraft);
    }
    mouse_grab_state = 0;
}

#include <SDL/SDL_events.h>

// Block UI Interaction When Mouse Is Locked
static int32_t Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection(unsigned char *minecraft) {
    if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) {
        // Call Original Method
        return (*Minecraft_isCreativeMode)(minecraft);
    } else {
        // Disable Item Drop Ticking
        return 1;
    }
}

// Block UI Interaction When Mouse Is Locked
static void Gui_handleClick_injection(unsigned char *this, int32_t param_2, int32_t param_3, int32_t param_4) {
    if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) {
        // Call Original Method
        (*Gui_handleClick)(this, param_2, param_3, param_4);
    }
}

// Store Left Click (0 = Not Pressed, 1 = Pressed, 2 = Repeat)
// This Is Set To Repeat After First Attempted Left-Click Build Interaction
static int is_left_click = 0;
void input_set_is_left_click(int val) {
    if ((is_left_click == 0 && val == 1) || (is_left_click != 0 && val == 0) || (is_left_click == 1 && val == 2)) {
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
        unsigned char *hit_result = minecraft + Minecraft_hit_result_property_offset;
        int32_t hit_result_type = *(int32_t *) (hit_result + HitResult_type_property_offset);
        // Check if The Target Is An Entity Using HitResult
        if (hit_result_type == 1) {
            // Change BuildActionIntention To Attack/Place Mode (Place Will Not Happen Because The HitResult Is An Entity)
            *build_action_intention_return = 0x8;
        }
        // Block Repeat Changes Without Releasing Left Click
        is_left_click = 2;
    }

    return ret;
}

void init_input() {
    // Disable Item Dropping Using The Cursor When Cursor Is Hidden
    overwrite_call((void *) 0x27800, Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection);
    // Disable Opening Inventory Using The Cursor When Cursor Is Hidden
    overwrite_calls((void *) Gui_handleClick, Gui_handleClick_injection);

    // Enable Bow & Arrow Fix
    fix_bow = feature_has("Fix Bow & Arrow");
    // Fix Bow & Arrow + Clear Unused Sign Input
    overwrite_calls((void *) Minecraft_tickInput, Minecraft_tickInput_injection);

    if (feature_has("Fix Attacking")) {
        // Allow Attacking Mobs
        patch_address(MouseBuildInput_tickBuild_vtable_addr, (void *) MouseBuildInput_tickBuild_injection);
    }

    // Init C++
    init_input_cpp();
}