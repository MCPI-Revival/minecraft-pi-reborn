#include <libcore/libcore.h>

#include "game_mode.h"

#include "../minecraft.h"

// Get Minecraft From Screen
static unsigned char *get_minecraft_from_screen(unsigned char *screen) {
    return *(unsigned char **) (screen + 0x14);
}

// Redirect Create World Button To SimpleLevelChooseScreen
#define WORLD_NAME "world"
#define SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE 0x68
static void SelectWorldScreen_tick_injection(unsigned char *screen) {
    bool create_world = *(bool *) (screen + 0xfc);
    if (create_world) {
        // Get New World Name
        std::string new_name;
        (*SelectWorldScreen_getUniqueLevelName)(new_name, screen, WORLD_NAME);
        // Create SimpleLevelChooseScreen
        unsigned char *new_screen = (unsigned char *) ::operator new(SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE);
        (*SimpleChooseLevelScreen)(new_screen, new_name);
        // Set Screen
        unsigned char *minecraft = get_minecraft_from_screen(screen);
        (*Minecraft_setScreen)(minecraft, new_screen);
        // Finish
        *(bool *) (screen + 0xf9) = true;
    } else {
        (*SelectWorldScreen_tick)(screen);
    }
}
static void Touch_SelectWorldScreen_tick_injection(unsigned char *screen) {
    bool create_world = *(bool *) (screen + 0x154);
    if (create_world) {
        // Get New World Name
        std::string new_name;
        (*Touch_SelectWorldScreen_getUniqueLevelName)(new_name, screen, WORLD_NAME);
        // Create SimpleLevelChooseScreen
        unsigned char *new_screen = (unsigned char *) ::operator new(SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE);
        (*SimpleChooseLevelScreen)(new_screen, new_name);
        // Set Screen
        unsigned char *minecraft = get_minecraft_from_screen(screen);
        (*Minecraft_setScreen)(minecraft, new_screen);
        // Finish
        *(bool *) (screen + 0x151) = true;
    } else {
        (*Touch_SelectWorldScreen_tick)(screen);
    }
}

void init_game_mode_cpp() {
    // Hijack Create World Button
    patch_address(SelectWorldScreen_tick_vtable_addr, (void *) SelectWorldScreen_tick_injection);
    patch_address(Touch_SelectWorldScreen_tick_vtable_addr, (void *) Touch_SelectWorldScreen_tick_injection);
    // Make The SimpleChooseLevelScreen Back Button Go To SelectWorldScreen Instead Of StartMenuScreen
    unsigned char simple_choose_level_screen_back_button_patch[4] = {0x05, 0x10, 0xa0, 0xe3};
    patch((void *) 0x31144, simple_choose_level_screen_back_button_patch);
}