#include <libreborn/libreborn.h>

#include "game-mode.h"

#include <symbols/minecraft.h>

// Get Minecraft From Screen
static unsigned char *get_minecraft_from_screen(unsigned char *screen) {
    return *(unsigned char **) (screen + Screen_minecraft_property_offset);
}

// Redirect Create World Button To SimpleLevelChooseScreen
#define WORLD_NAME "world"
static void SelectWorldScreen_tick_injection(unsigned char *screen) {
    bool create_world = *(bool *) (screen + SelectWorldScreen_should_create_world_property_offset);
    if (create_world) {
        // Get New World Name
        std::string new_name = (*SelectWorldScreen_getUniqueLevelName)(screen, WORLD_NAME);
        // Create SimpleLevelChooseScreen
        unsigned char *new_screen = (unsigned char *) ::operator new(SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE);
        ALLOC_CHECK(new_screen);
        (*SimpleChooseLevelScreen)(new_screen, new_name);
        // Set Screen
        unsigned char *minecraft = get_minecraft_from_screen(screen);
        (*Minecraft_setScreen)(minecraft, new_screen);
        // Finish
        *(bool *) (screen + SelectWorldScreen_world_created_property_offset) = true;
    } else {
        (*SelectWorldScreen_tick)(screen);
    }
}
static void Touch_SelectWorldScreen_tick_injection(unsigned char *screen) {
    bool create_world = *(bool *) (screen + Touch_SelectWorldScreen_should_create_world_property_offset);
    if (create_world) {
        // Get New World Name
        std::string new_name = (*Touch_SelectWorldScreen_getUniqueLevelName)(screen, WORLD_NAME);
        // Create SimpleLevelChooseScreen
        unsigned char *new_screen = (unsigned char *) ::operator new(SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE);
        ALLOC_CHECK(new_screen);
        (*SimpleChooseLevelScreen)(new_screen, new_name);
        // Set Screen
        unsigned char *minecraft = get_minecraft_from_screen(screen);
        (*Minecraft_setScreen)(minecraft, new_screen);
        // Finish
        *(bool *) (screen + Touch_SelectWorldScreen_world_created_property_offset) = true;
    } else {
        (*Touch_SelectWorldScreen_tick)(screen);
    }
}

void _init_game_mode_cpp() {
    // Hijack Create World Button
    patch_address(SelectWorldScreen_tick_vtable_addr, (void *) SelectWorldScreen_tick_injection);
    patch_address(Touch_SelectWorldScreen_tick_vtable_addr, (void *) Touch_SelectWorldScreen_tick_injection);
    // Make The SimpleChooseLevelScreen Back Button Go To SelectWorldScreen Instead Of StartMenuScreen
    unsigned char simple_choose_level_screen_back_button_patch[4] = {0x05, 0x10, 0xa0, 0xe3}; // "mov r1, #0x5"
    patch((void *) 0x31144, simple_choose_level_screen_back_button_patch);
    patch((void *) 0x3134c, simple_choose_level_screen_back_button_patch);
}
