#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/classic-ui/classic-ui.h>

// Classic HUD
#define DEFAULT_HUD_PADDING 2
#define NEW_HUD_PADDING 1
#define HUD_ELEMENT_WIDTH 82
#define HUD_ELEMENT_HEIGHT 9
#define TOOLBAR_HEIGHT 22
#define SLOT_WIDTH 20
#define DEFAULT_BUBBLES_PADDING 1
#define NUMBER_OF_SLOTS 9
static bool use_classic_hud = false;
static void Gui_renderHearts_GuiComponent_blit_hearts_injection(GuiComponent *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    const Minecraft *minecraft = ((Gui *) component)->minecraft;
    x_dest -= DEFAULT_HUD_PADDING;
    const float width = float(minecraft->screen_width) * Gui::InvGuiScale;
    const float height = float(minecraft->screen_height) * Gui::InvGuiScale;
    x_dest += int32_t(width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += int32_t(height) - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    component->blit(x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
GuiComponent_blit_t get_blit_with_classic_hud_offset() {
    return use_classic_hud ? Gui_renderHearts_GuiComponent_blit_hearts_injection : GuiComponent_blit->get(false);
}
static void Gui_renderHearts_GuiComponent_blit_armor_injection(Gui *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    const Minecraft *minecraft = component->minecraft;
    x_dest -= DEFAULT_HUD_PADDING + HUD_ELEMENT_WIDTH;
    const float width = float(minecraft->screen_width) * Gui::InvGuiScale;
    const float height = float(minecraft->screen_height) * Gui::InvGuiScale;
    x_dest += int32_t(width) - ((int32_t(width) - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2) - HUD_ELEMENT_WIDTH;
    y_dest -= DEFAULT_HUD_PADDING;
    y_dest += int32_t(height) - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    component->blit(x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
static void Gui_renderBubbles_GuiComponent_blit_injection(Gui *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src) {
    const Minecraft *minecraft = component->minecraft;
    x_dest -= DEFAULT_HUD_PADDING;
    const float width = float(minecraft->screen_width) * Gui::InvGuiScale;
    const float height = float(minecraft->screen_height) * Gui::InvGuiScale;
    x_dest += int32_t(width - (NUMBER_OF_SLOTS * SLOT_WIDTH)) / 2;
    y_dest -= DEFAULT_HUD_PADDING + DEFAULT_BUBBLES_PADDING + HUD_ELEMENT_HEIGHT;
    y_dest += int32_t(height) - HUD_ELEMENT_HEIGHT - TOOLBAR_HEIGHT - HUD_ELEMENT_HEIGHT - NEW_HUD_PADDING;
    // Call Original Method
    component->blit(x_dest, y_dest, x_src, y_src, width_dest, height_dest, width_src, height_src);
}
int get_classic_hud_y_offset(Minecraft *minecraft) {
    int ret = 0;
    if (use_classic_hud && !minecraft->isCreativeMode()) {
        ret += (HUD_ELEMENT_HEIGHT * 2) + NEW_HUD_PADDING;
    }
    return ret;
}

// Classic Slot Count Location
static void Gui_renderSlotText_injection_common(Gui *self, const ItemInstance *item, float x, float y, const bool param_1, const bool param_2) {
    // Position
    x += 17;
    y += 9;
    // Call Original Method
    self->renderSlotText(item, x, y, param_1, param_2);
}
static void Gui_renderSlotText_injection_furnace(Gui *self, const ItemInstance *item, float x, float y, const bool param_1, const bool param_2) {
    // Position
    x += 4;
    y += 5;
    // Call Original Method
    Gui_renderSlotText_injection_common(self, item, x, y, param_1, param_2);
}
static void unscale_slot_text(float &x, float &y) {
    const float factor = 0.5f * Gui::GuiScale;
    x /= factor;
    y /= factor;
}
static void Gui_renderSlotText_injection_classic_inventory(Gui *self, const ItemInstance *item, float x, float y, const bool param_1, const bool param_2) {
    // Position
    unscale_slot_text(x, y);
    // Call Original Method
    Gui_renderSlotText_injection_common(self, item, x, y, param_1, param_2);
}
static void Gui_renderSlotText_injection_toolbar(Gui *self, const ItemInstance *item, float x, float y, const bool param_1, const bool param_2) {
    // Position
    y--;
    unscale_slot_text(x, y);
    // Call Original Method
    Gui_renderSlotText_injection_common(self, item, x, y, param_1, param_2);
}
static void Gui_renderSlotText_injection_inventory(Gui *self, const ItemInstance *item, float x, float y, const bool param_1, const bool param_2) {
    // Position
    unscale_slot_text(x, y);
    x++;
    y++;
    // Call Original Method
    Gui_renderSlotText_injection_common(self, item, x, y, param_1, param_2);
}
template <auto *const func>
static void Gui_renderSlotText_Font_draw_injection(Font *self, const char *raw_string, float x, float y, uint color) {
    // Fix X
    std::string string = raw_string;
    x -= self->width(string);
    // Fix Color
    if (color == 0xffcccccc) {
        color = 0xffffffff;
    }
    // Call
    (*func)->get(false)(self, string, x, y, color);
}

// Init
void init_classic_ui() {
    // Classic HUD
    if (feature_has("Classic HUD", server_disabled)) {
        use_classic_hud = true;
        overwrite_call((void *) 0x26758, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x2656c, (void *) Gui_renderHearts_GuiComponent_blit_armor_injection);
        overwrite_call((void *) 0x268c4, (void *) Gui_renderBubbles_GuiComponent_blit_injection);
        overwrite_call((void *) 0x266f8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
        overwrite_call((void *) 0x267c8, (void *) Gui_renderHearts_GuiComponent_blit_hearts_injection);
    }

    // Classic Slot Count Location
    if (feature_has("Classic Item Count UI", server_disabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x27074, nop_patch);
        patch((void *) 0x33984, nop_patch);
        patch((void *) 0x1e424, nop_patch);
        overwrite_call((void *) 0x1e4b8, (void *) Gui_renderSlotText_injection_inventory);
        overwrite_call((void *) 0x27100, (void *) Gui_renderSlotText_injection_toolbar);
        overwrite_call((void *) 0x339b4, (void *) Gui_renderSlotText_injection_classic_inventory);
        overwrite_call((void *) 0x2b268, (void *) Gui_renderSlotText_injection_furnace);
        overwrite_call((void *) 0x320c4, (void *) Gui_renderSlotText_injection_furnace);
        overwrite_call((void *) 0x25e84, (void *) Gui_renderSlotText_Font_draw_injection<&Font_draw>);
        overwrite_call((void *) 0x25e74, (void *) Gui_renderSlotText_Font_draw_injection<&Font_drawShadow>);
    }
}