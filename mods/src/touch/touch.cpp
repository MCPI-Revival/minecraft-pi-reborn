#include <libreborn/libreborn.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>

#include <symbols/minecraft.h>

// Enable Touch GUI
static int32_t Minecraft_isTouchscreen_injection(__attribute__((unused)) unsigned char *minecraft) {
    return 1;
}

// IngameBlockSelectionScreen Memory Allocation Override
static unsigned char *operator_new_IngameBlockSelectionScreen_injection(__attribute__((unused)) uint32_t size) {
    return (unsigned char *) ::operator new(TOUCH_INGAME_BLOCK_SELECTION_SCREEN_SIZE);
}

// Improved Button Hover Behavior
static int32_t Button_hovered_injection(__attribute__((unused)) unsigned char *button, __attribute__((unused)) unsigned char *minecraft, __attribute__((unused)) int32_t click_x, __attribute__((unused)) int32_t click_y) {
    // Get Mouse Position
    int32_t x = (*Mouse_getX)() * (*InvGuiScale);
    int32_t y = (*Mouse_getY)() * (*InvGuiScale);

    // Get Button Position
    int32_t button_x1 = *(int32_t *) (button + Button_x_property_offset);
    int32_t button_y1 = *(int32_t *) (button + Button_y_property_offset);
    int32_t button_x2 = button_x1 + (*(int32_t *) (button + Button_width_property_offset));
    int32_t button_y2 = button_y1 + (*(int32_t *) (button + Button_height_property_offset));

    // Check
    return x >= button_x1 && x <= button_x2 && y >= button_y1 && y <= button_y2;
}
static void LargeImageButton_render_GuiComponent_drawCenteredString_injection(unsigned char *component, unsigned char *font, std::string const& text, int32_t x, int32_t y, int32_t color) {
    // Change Color On Hover
    if (color == 0xe0e0e0 && Button_hovered_injection(component, NULL, 0, 0)) {
        color = 0xffffa0;
    }

    // Call Original Method
    (*GuiComponent_drawCenteredString)(component, font, text, x, y, color);
}

// Init
void init_touch() {
    int touch_gui = feature_has("Full Touch GUI", server_disabled);
    int touch_buttons = touch_gui;
    if (touch_gui) {
        // Main UI
        overwrite((void *) Minecraft_isTouchscreen, (void *) Minecraft_isTouchscreen_injection);

        // Force Correct Toolbar Size
        unsigned char toolbar_patch[4] = {0x01, 0x00, 0x50, 0xe3}; // "cmp r0, #0x1"
        patch((void *) 0x257b0, toolbar_patch);
    } else {
        // Force Touch Inventory
        if (feature_has("Force Touch GUI Inventory", server_disabled)) {
            overwrite_call((void *) 0x2943c, (void *) operator_new_IngameBlockSelectionScreen_injection);
            overwrite_call((void *) 0x29444, (void *) Touch_IngameBlockSelectionScreen);
            // Make "Craft" And "Armor" Buttons Use Classic GUI Style (Button And TButton Have The Same Size)
            overwrite_call((void *) 0x3b060, (void *) Button);
            overwrite_call((void *) 0x3b08c, (void *) Button);
        }

        // Force Touch Button Behavior
        if (feature_has("Force Touch GUI Button Behavior", server_disabled)) {
            touch_buttons = 1;
            overwrite_call((void *) 0x1baf4, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x1be40, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x1c470, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x1e868, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x290b8, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x29168, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x3e314, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x2cbc0, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x2ea7c, (void *) Minecraft_isTouchscreen_injection);
            overwrite_call((void *) 0x4a438, (void *) Minecraft_isTouchscreen_injection);
        }
    }

    // Improved Button Hover Behavior
    if (touch_buttons && feature_has("Improved Button Hover Behavior", server_disabled)) {
        overwrite((void *) Button_hovered, (void *) Button_hovered_injection);
        overwrite_call((void *) 0x1ebd4, (void *) LargeImageButton_render_GuiComponent_drawCenteredString_injection);
    }

    // Show Block Outlines
    int block_outlines = feature_has("Show Block Outlines", server_disabled);
    unsigned char outline_patch[4] = {(unsigned char) (block_outlines ? !touch_gui : touch_gui), 0x00, 0x50, 0xe3}; // "cmp r0, #0x1" or "cmp r0, #0x0"
    patch((void *) 0x4a210, outline_patch);
}
