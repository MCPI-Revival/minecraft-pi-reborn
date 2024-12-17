#include <cstdint>

#include <libreborn/patch.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/touch/touch.h>

#include <symbols/minecraft.h>

// Enable Touch GUI
static bool Minecraft_isTouchscreen_call_injection(__attribute__((unused)) Minecraft *minecraft) {
    return true;
}
static bool Minecraft_isTouchscreen_injection(__attribute__((unused)) Minecraft_isTouchscreen_t original, __attribute__((unused)) Minecraft *minecraft) {
    return Minecraft_isTouchscreen_call_injection(minecraft);
}

// IngameBlockSelectionScreen Memory Allocation Override
static unsigned char *operator_new_IngameBlockSelectionScreen_injection(__attribute__((unused)) uint32_t size) {
    return (unsigned char *) ::operator new(sizeof(Touch_IngameBlockSelectionScreen));
}

// Improved Button Hover Behavior
static int32_t Button_hovered_injection(__attribute__((unused)) Button_hovered_t original, __attribute__((unused)) Button *button, __attribute__((unused)) Minecraft *minecraft, __attribute__((unused)) int32_t click_x, __attribute__((unused)) int32_t click_y) {
    // Get Mouse Position
    const int32_t x = Mouse::getX() * Gui::InvGuiScale;
    const int32_t y = (Mouse::getY() * Gui::InvGuiScale) - 1; // Screen::mouseEvent Offsets Mouse Events

    // Get Button Position
    const int32_t button_x1 = button->x;
    const int32_t button_y1 = button->y;
    const int32_t button_x2 = button_x1 + button->width;
    const int32_t button_y2 = button_y1 + button->height;

    // Check
    return x >= button_x1 && x < button_x2 && y >= button_y1 && y < button_y2;
}
static void LargeImageButton_render_GuiComponent_drawCenteredString_injection(GuiComponent *component, Font *font, const std::string &text, int32_t x, int32_t y, uint32_t color) {
    // Change Color On Hover
    if (color == 0xe0e0e0 && Button_hovered_injection(nullptr, (Button *) component, nullptr, 0, 0)) {
        color = 0xffffa0;
    }

    // Call Original Method
    component->drawCenteredString(font, text, x, y, color);
}

// Create Button
int touch_gui = 0;
template <typename T>
static Button *create_button(int id, std::string text) {
    T *button = T::allocate();
    button->constructor(id, text);
    return (Button *) button;
}
Button *touch_create_button(const int id, const std::string &text) {
    if (touch_gui) {
        return create_button<Touch_TButton>(id, text);
    } else {
        return create_button<Button>(id, text);
    }
}

// Init
void init_touch() {
    touch_gui = feature_has("Full Touch UI", server_disabled);
    bool touch_buttons = touch_gui;
    if (touch_gui) {
        // Main UI
        overwrite_calls(Minecraft_isTouchscreen, Minecraft_isTouchscreen_injection);

        // Force Correct Toolbar Size
        unsigned char toolbar_patch[4] = {0x01, 0x00, 0x50, 0xe3}; // "cmp r0, #0x1"
        patch((void *) 0x257b0, toolbar_patch);
    } else {
        // Force Touch Inventory
        if (feature_has("Force Touch UI Inventory", server_disabled)) {
            overwrite_call_manual((void *) 0x2943c, (void *) operator_new_IngameBlockSelectionScreen_injection);
            overwrite_call_manual((void *) 0x29444, (void *) Touch_IngameBlockSelectionScreen_constructor->get(true));
            // Make "Craft" And "Armor" Buttons Use Classic GUI Style (Button And TButton Have The Same Size)
            overwrite_call_manual((void *) 0x3b060, (void *) Button_constructor->get(true));
            overwrite_call_manual((void *) 0x3b08c, (void *) Button_constructor->get(true));
        }

        // Force Touch Button Behavior
        if (feature_has("Force Touch UI Button Behavior", server_disabled)) {
            touch_buttons = true;
            overwrite_call((void *) 0x1baf4, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x1be40, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x1c470, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x1e868, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x290b8, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x29168, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x3e314, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x2cbc0, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x2ea7c, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
            overwrite_call((void *) 0x4a438, Minecraft_isTouchscreen, Minecraft_isTouchscreen_call_injection);
        }
    }

    // Improved Button Hover Behavior
    if (touch_buttons && feature_has("Improved Button Hover Behavior", server_disabled)) {
        overwrite_calls(Button_hovered, Button_hovered_injection);
        overwrite_call((void *) 0x1ebd4, GuiComponent_drawCenteredString, LargeImageButton_render_GuiComponent_drawCenteredString_injection);
    }

    // Show Block Outlines
    const bool block_outlines = feature_has("Show Block Outlines", server_disabled);
    unsigned char outline_patch[4] = {(unsigned char) (block_outlines ? !touch_gui : touch_gui), 0x00, 0x50, 0xe3}; // "cmp r0, #0x1" or "cmp r0, #0x0"
    patch((void *) 0x4a210, outline_patch);
}
