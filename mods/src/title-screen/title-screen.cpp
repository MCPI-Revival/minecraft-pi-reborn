#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/compat/compat.h>

// Improved Title Screen Background
static void StartMenuScreen_render_Screen_renderBackground_injection(StartMenuScreen *screen) {
    // Draw
    Minecraft *minecraft = screen->minecraft;
    Textures *textures = minecraft->textures;
    std::string texture = "gui/titleBG.png";
    (*Textures_loadAndBindTexture)(textures, &texture);
    (*StartMenuScreen_blit)(screen, 0, 0, 0, 0, screen->width, screen->height, 0x100, 0x100);
}

// Add Buttons Back To Classic Start Screen
static void StartMenuScreen_init_injection(StartMenuScreen *screen) {
    // Call Original Method
    (*StartMenuScreen_init_non_virtual)(screen);

    // Add Button
    std::vector<Button *> *rendered_buttons = &screen->rendered_buttons;
    std::vector<Button *> *selectable_buttons = &screen->selectable_buttons;
    Button *options_button = &screen->options_button;
    rendered_buttons->push_back(options_button);
    selectable_buttons->push_back(options_button);
    Button *create_button = &screen->create_button; // Repurpose Unused "Create" Button As Quit Button
    rendered_buttons->push_back(create_button);
    selectable_buttons->push_back(create_button);
}

// Add Functionality To Quit Button
static void StartMenuScreen_buttonClicked_injection(StartMenuScreen *screen, Button *button) {
    Button *quit_button = &screen->create_button;
    if (button == quit_button) {
        // Quit
        compat_request_exit();
    } else {
        // Call Original Method
        (*StartMenuScreen_buttonClicked_non_virtual)(screen, button);
    }
}

// Init
void init_title_screen() {
    // Improved Title Screen Background
    if (feature_has("Add Title Screen Background", server_disabled)) {
        // Switch Background
        overwrite_call((void *) 0x39528, (void *) StartMenuScreen_render_Screen_renderBackground_injection);
        overwrite_call((void *) 0x3dee0, (void *) StartMenuScreen_render_Screen_renderBackground_injection);
        // Text Color
        patch_address((void *) 0x397ac, (void *) 0xffffffff);
        patch_address((void *) 0x3e10c, (void *) 0xffffffff);
    }

    // Improved Classic Title Screen
    if (feature_has("Improved Classic Title Screen", server_disabled)) {
        // Add Options Button Back To Classic Start Screen
        patch_address(StartMenuScreen_init_vtable_addr, (void *) StartMenuScreen_init_injection);

        // Fix Classic UI Button Size
        unsigned char classic_button_height_patch[4] = {0x18, 0x30, 0xa0, 0xe3}; // "mov r3, #0x18"
        patch((void *) 0x39a9c, classic_button_height_patch);
        patch((void *) 0x39ae0, classic_button_height_patch);

        // Fix Classic UI Buttons Spacing
        {
            // Join Button
            unsigned char classic_join_button_spacing_patch[4] = {0x12, 0x20, 0x83, 0xe2}; // "add r2, r3, #0x12"
            patch((void *) 0x39894, classic_join_button_spacing_patch);
            // Start Button
            unsigned char classic_start_button_spacing_patch[4] = {0x08, 0x20, 0x43, 0xe2}; // "sub r2, r3, #0x08"
            patch((void *) 0x3988c, classic_start_button_spacing_patch);
            // Options Button
            unsigned char classic_options_button_spacing_patch[4] = {0x2c, 0x30, 0x83, 0xe2}; // "add r3, r3, #0x2c"
            patch((void *) 0x39898, classic_options_button_spacing_patch);
        }

        // Rename "Create" Button To "Quit"
        patch_address((void *) Strings_classic_create_button_text, (void *) "Quit");

        // Add Functionality To Quit Button
        patch_address(StartMenuScreen_buttonClicked_vtable_addr, (void *) StartMenuScreen_buttonClicked_injection);
    }
}
