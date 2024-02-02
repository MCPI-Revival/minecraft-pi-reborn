#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "input-internal.h"
#include <mods/input/input.h>
#include <mods/feature/feature.h>
#include <mods/creative/creative.h>

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
static void _handle_back(Minecraft *minecraft) {
    // If Minecraft's Level property is initialized, but Minecraft's Player property is NULL, then Minecraft::handleBack may crash.
    if (minecraft->level != NULL && minecraft->player == NULL) {
        // Unable to safely run Minecraft::handleBack, deferring until safe.
        return;
    }
    // Send Event
    for (int i = 0; i < back_button_presses; i++) {
        minecraft->vtable->handleBack(minecraft, 0);
    }
    back_button_presses = 0;
}

// Fix OptionsScreen Ignoring The Back Button
static bool OptionsScreen_handleBackEvent_injection(OptionsScreen *screen, bool do_nothing) {
    if (!do_nothing) {
        Minecraft *minecraft = screen->minecraft;
        Minecraft_setScreen(minecraft, NULL);
    }
    return 1;
}

// Fix "Sleeping Beauty" Bug
static int32_t InBedScreen_handleBackEvent_injection(InBedScreen *screen, bool do_nothing) {
    if (!do_nothing) {
        // Close Screen
        Minecraft *minecraft = screen->minecraft;
        Minecraft_setScreen(minecraft, NULL);
        // Stop Sleeping
        LocalPlayer *player = minecraft->player;
        if (player != NULL) {
            player->vtable->stopSleepInBed(player, 1, 1, 1);
        }
    }
    return 1;
}

// Set Mouse Grab State
static int mouse_grab_state = 0;
void input_set_mouse_grab_state(int state) {
    mouse_grab_state = state;
}

// Grab/Un-Grab Mouse
static void _handle_mouse_grab(Minecraft *minecraft) {
    if (mouse_grab_state == -1) {
        // Grab
        Minecraft_grabMouse(minecraft);
    } else if (mouse_grab_state == 1) {
        // Un-Grab
        Minecraft_releaseMouse(minecraft);
    }
    mouse_grab_state = 0;
}

#include <SDL/SDL.h>

// Block UI Interaction When Mouse Is Locked
static bool Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection(Minecraft *minecraft) {
    bool is_in_game = minecraft->screen == NULL || minecraft->screen->vtable == (Screen_vtable *) Touch_IngameBlockSelectionScreen_vtable_base;
    if (!enable_misc || (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF && is_in_game)) {
        // Call Original Method
        return creative_is_restricted() && Minecraft_isCreativeMode(minecraft);
    } else {
        // Disable Item Drop Ticking
        return 1;
    }
}

// Block UI Interaction When Mouse Is Locked
static void Gui_handleClick_injection(Gui *gui, int32_t param_2, int32_t param_3, int32_t param_4) {
    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
        // Call Original Method
        Gui_handleClick(gui, param_2, param_3, param_4);
    }
}

// Init
void _init_misc() {
    enable_misc = feature_has("Miscellaneous Input Fixes", server_disabled);
    if (enable_misc) {
        // Fix OptionsScreen Ignoring The Back Button
        patch_address(OptionsScreen_handleBackEvent_vtable_addr, (void *) OptionsScreen_handleBackEvent_injection);
        // Fix "Sleeping Beauty" Bug
        patch_address(InBedScreen_handleBackEvent_vtable_addr, (void *) InBedScreen_handleBackEvent_injection);
        // Disable Opening Inventory Using The Cursor When Cursor Is Hidden
        overwrite_calls((void *) Gui_handleClick, (void *) Gui_handleClick_injection);
    }
    // Disable Item Dropping Using The Cursor When Cursor Is Hidden
    overwrite_call((void *) 0x27800, (void *) Gui_tickItemDrop_Minecraft_isCreativeMode_call_injection);

    input_run_on_tick(_handle_back);
    input_run_on_tick(_handle_mouse_grab);
}
