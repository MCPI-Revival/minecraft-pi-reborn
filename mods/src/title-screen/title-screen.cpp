#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/compat/compat.h>

// Improved Title Screen Background
static void StartMenuScreen_render_Screen_renderBackground_injection(unsigned char *screen) {
    // Draw
    unsigned char *minecraft = *(unsigned char **) (screen + Screen_minecraft_property_offset);
    unsigned char *textures = *(unsigned char **) (minecraft + Minecraft_textures_property_offset);
    (*Textures_loadAndBindTexture)(textures, "gui/titleBG.png");
    (*GuiComponent_blit)(screen, 0, 0, 0, 0, *(int32_t *) (screen + Screen_width_property_offset), *(int32_t *) (screen + Screen_height_property_offset), 0x100, 0x100);
}

// Add Buttons Back To Classic Start Screen
static void StartMenuScreen_init_injection(unsigned char *screen) {
    // Call Original Method
    (*StartMenuScreen_init)(screen);

    // Add Button
    std::vector<unsigned char *> *rendered_buttons = (std::vector<unsigned char *> *) (screen + Screen_rendered_buttons_property_offset);
    std::vector<unsigned char *> *selectable_buttons = (std::vector<unsigned char *> *) (screen + Screen_selectable_buttons_property_offset);
    unsigned char *options_button = screen + StartMenuScreen_options_button_property_offset; // Repurpose Unused "Options" Button As Quit Button
    rendered_buttons->push_back(options_button);
    selectable_buttons->push_back(options_button);
}

// Add Functionality To Quit Button
static void StartMenuScreen_buttonClicked_injection(unsigned char *screen, unsigned char *button) {
    unsigned char *quit_button = screen + StartMenuScreen_options_button_property_offset;
    if (button == quit_button) {
        // Quit
        compat_request_exit();
    } else {
        // Call Original Method
        (*StartMenuScreen_buttonClicked)(screen, button);
    }
}

// Adjust Touch Title Screen Button Positions
static void Touch_StartMenuScreen_setupPositions_injection(unsigned char *screen) {
    // Call Original Method
    (*Touch_StartMenuScreen_setupPositions)(screen);

    // Get Width
    int32_t width = *(int32_t *) (screen + Screen_width_property_offset);

    // Move Buttons
    unsigned char *join_game_button = screen + Touch_StartMenuScreen_join_game_button_property_offset;
    int32_t join_game_button_width = *(int32_t *) (join_game_button + Button_width_property_offset);
    unsigned char *start_game_button = screen + Touch_StartMenuScreen_start_game_button_property_offset;
    int32_t start_game_button_width = *(int32_t *) (start_game_button + Button_width_property_offset);
    int32_t padding = (width - join_game_button_width - start_game_button_width) / 3;
    *(int32_t *) (join_game_button + Button_x_property_offset) = padding;
    *(int32_t *) (start_game_button + Button_x_property_offset) = (padding * 2) + join_game_button_width;
}

// Init
void init_title_screen() {
    // Improved Title Screen Background
    if (feature_has("Add Title Screen Background", server_disabled)) {
        // Switch Background
        overwrite_call((void *) 0x4a40c, (void *) StartMenuScreen_render_Screen_renderBackground_injection);
        overwrite_call((void *) 0x52360, (void *) StartMenuScreen_render_Screen_renderBackground_injection);
        // Text Color
        patch_address((void *) 0x4a764, (void *) 0xffffffff);
        patch_address((void *) 0x525f4, (void *) 0xffffffff);
    }

    // Improved Classic Title Screen
    if (feature_has("Improved Classic Title Screen", server_disabled)) {
        // Add Options Button Back To Classic Start Screen
        patch_address(StartMenuScreen_init_vtable_addr, (void *) StartMenuScreen_init_injection);

        // Fix Classic UI Button Size
        unsigned char classic_button_height_patch[4] = {0x18, 0xc0, 0xa0, 0xe3}; // "mov r12, #0x18"
        patch((void *) 0x4ac18, classic_button_height_patch);
        unsigned char classic_button_width_patch[4] = {0xa0, 0x70, 0xa0, 0xe3}; // "mov r7, #0xa0"
        patch((void *) 0x4ac14, classic_button_width_patch);

        // Fix Classic UI Buttons Spacing
        {
            // Join Button
            unsigned char classic_join_button_spacing_patch[4] = {0x12, 0x00, 0x80, 0xe2}; // "add r0, r0, #0x12"
            patch((void *) 0x4a380, classic_join_button_spacing_patch);
            // Start Button
            unsigned char classic_start_button_spacing_patch[4] = {0x08, 0x70, 0x40, 0xe2}; // "sub r7, r0, #0x08"
            patch((void *) 0x4a378, classic_start_button_spacing_patch);
            // Options Button
            unsigned char classic_options_button_spacing_patch[4] = {0x2c, 0x20, 0x80, 0xe2}; // "add r3, r3, #0x2c"
            patch((void *) 0x4a370, classic_options_button_spacing_patch);
        }

        // Rename "Options" Button To "Quit"
        patch_address((void *) classic_options_button_text, (void *) "Quit");

        // Add Functionality To Quit Button
        patch_address(StartMenuScreen_buttonClicked_vtable_addr, (void *) StartMenuScreen_buttonClicked_injection);
    }

    // Improved Touch Title Screen
    if (feature_has("Improved Touch Title Screen", server_disabled)) {
        // Remove Redundant Options Button
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x53470, nop_patch);

        // Adjust Touch Title Screen Button Positions
        patch_address(Touch_StartMenuScreen_setupPositions_vtable_addr, (void *) Touch_StartMenuScreen_setupPositions_injection);
    }
}
