#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "input.h"
#include "../feature/feature.h"
#include "../creative/creative.h"

// Enable Miscellaneous Input Fixes
static int enable_misc = 0;

// Store Back Button Presses
static int back_button_presses = 0;
int input_back() {
    if (enable_misc) {
        back_button_presses++;
        return 1; // Handled
    } else {
        return 0; // Not Handled
    }
}

// Handle Back Button Presses
static void _handle_back(unsigned char *minecraft) {
    unsigned char *minecraft_vtable = *(unsigned char **) minecraft;
    Minecraft_handleBack_t Minecraft_handleBack = *(Minecraft_handleBack_t *) (minecraft_vtable + Minecraft_handleBack_vtable_offset);
    for (int i = 0; i < back_button_presses; i++) {
        (*Minecraft_handleBack)(minecraft, 0);
    }
    back_button_presses = 0;
}

// Fix OptionsScreen Ignoring The Back Button
static int32_t OptionsScreen_handleBackEvent_injection(unsigned char *screen, bool do_nothing) {
    if (!do_nothing) {
        unsigned char *minecraft = *(unsigned char **) (screen + Screen_minecraft_property_offset);
        (*Minecraft_setScreen)(minecraft, NULL);
    }
    return 1;
}

// Set Mouse Grab State
static int mouse_grab_state = 0;
void input_set_mouse_grab_state(int state) {
    mouse_grab_state = state;
}

// Grab/Un-Grab Mouse
static void _handle_mouse_grab(unsigned char *minecraft) {
    if (mouse_grab_state == -1) {
        // Grab
        (*Minecraft_grabMouse)(minecraft);
    } else if (mouse_grab_state == 1) {
        // Un-Grab
        (*Minecraft_releaseMouse)(minecraft);
    }
    mouse_grab_state = 0;
}

#include <SDL/SDL.h>

// Block UI Interaction When Mouse Is Locked
static bool Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection(unsigned char *minecraft) {
    if (!enable_misc || SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Call Original Method
        return creative_is_restricted() && (*Minecraft_isCreativeMode)(minecraft);
    } else {
        // Disable Item Drop Ticking
        return 1;
    }
}

// Block UI Interaction When Mouse Is Locked
static void Gui_handleClick_injection(unsigned char *gui, int32_t param_2, int32_t param_3, int32_t param_4) {
    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Call Original Method
        (*Gui_handleClick)(gui, param_2, param_3, param_4);
    }
}

// Init
void _init_misc() {
    enable_misc = feature_has("Miscellaneous Input Fixes", 0);
    if (enable_misc) {
        // Fix OptionsScreen Ignoring The Back Button
        patch_address(OptionsScreen_handleBackEvent_vtable_addr, (void *) OptionsScreen_handleBackEvent_injection);
        // Disable Opening Inventory Using The Cursor When Cursor Is Hidden
        overwrite_calls((void *) Gui_handleClick, (void *) Gui_handleClick_injection);
    }
    // Disable Item Dropping Using The Cursor When Cursor Is Hidden
    overwrite_call((void *) 0x27800, (void *) Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection);

    input_run_on_tick(_handle_back);
    input_run_on_tick(_handle_mouse_grab);
}
