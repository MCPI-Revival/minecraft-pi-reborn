#include <fstream>
#include <cmath>

#include <libreborn/patch.h>
#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/compat/compat.h>
#include <mods/title-screen/title-screen.h>
#include <mods/misc/misc.h>

#include "internal.h"

// Improved Title Screen Background
template <typename Self>
static void StartMenuScreen_renderBackground_injection(Self *screen) {
    // Draw
    const Minecraft *minecraft = screen->minecraft;
    Textures *textures = minecraft->textures;
    std::string texture = "gui/titleBG.png";
    textures->loadAndBindTexture(texture);
    screen->blit(0, 0, 0, 0, screen->width, screen->height, 0x100, 0x100);
}

// Add Buttons Back To Classic Start Screen
static void StartMenuScreen_init_injection(StartMenuScreen_init_t original, StartMenuScreen *screen) {
    // Call Original Method
    original(screen);

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
static void StartMenuScreen_buttonClicked_injection(StartMenuScreen_buttonClicked_t original, StartMenuScreen *screen, Button *button) {
    const Button *quit_button = &screen->create_button;
    if (button == quit_button) {
        // Quit
        compat_request_exit();
    } else {
        // Call Original Method
        original(screen, button);
    }
}

// Fix High-Resolution Title
static constexpr int title_width = 256;
static constexpr int title_height = 64;
static Texture *StartMenuScreen_render_Textures_getTemporaryTextureData_injection(Textures *self, uint id) {
    // Call Original Method
    const Texture *out = self->getTemporaryTextureData(id);
    // Patch
    static Texture ret;
    ret = *out;
    ret.width = title_width;
    ret.height = title_height;
    return &ret;
}
static float StartMenuScreen_render_Mth_min_injection(__attribute__((unused)) float a, const float b) {
    return b;
}

// Track Version Text Y
int version_text_bottom;
static int (*adjust_version_y)(const StartMenuScreen *) = nullptr;
static void StartMenuScreen_render_GuiComponent_drawString_injection(GuiComponent *self, Font *font, const std::string &text, int x, int y, uint color) {
    // Adjust Position
    if (adjust_version_y) {
        y = adjust_version_y((StartMenuScreen *) self);
    }
    // Draw
    self->drawString(font, text, x, y, color);
    // Store Position
    version_text_bottom = y + line_height;
}

// Modern Logo
static constexpr float modern_title_scale = 0.75f;
static constexpr int modern_title_width = int(title_width * modern_title_scale);
static constexpr int modern_title_height = int(title_height * modern_title_scale);
static constexpr int version_text_y_offset = -6;
static int get_title_y(const StartMenuScreen *screen) {
    float y = float(screen->start_button.y - modern_title_height);
    y *= (5.0f / 24.0f);
    y = ceilf(y);
    return int(y);
}
static int get_version_y(const StartMenuScreen *screen) {
    int y = get_title_y(screen);
    y += modern_title_height;
    y += version_text_y_offset;
    return y;
}
static void StartMenuScreen_render_Screen_renderBackground_injection(StartMenuScreen *self) {
    // Call Original Method
    self->renderBackground();
    // Draw Logo
    self->minecraft->textures->loadAndBindTexture(Strings::title_texture_classic);
    const float x = float(self->width) / 2;
    const float y = float(get_title_y(self));
    constexpr int w = modern_title_width / 2;
    constexpr int h = modern_title_height;
    Tesselator &t = Tesselator::instance;
    media_glColor4f(1, 1, 1, 1);
    t.begin(GL_QUADS);
    t.vertexUV(x - w, y + h, self->z, 0, 1);
    t.vertexUV(x + w, y + h, self->z, 1, 1);
    t.vertexUV(x + w, y, self->z, 1, 0);
    t.vertexUV(x - w, y, self->z, 0, 0);
    t.draw();
}
static Texture *StartMenuScreen_render_Textures_getTemporaryTextureData_injection_modern(__attribute__((unused)) Textures *self, __attribute__((unused)) uint id) {
    return nullptr;
}

// Init
void init_title_screen() {
    // Improved Title Screen Background
    if (feature_has("Add Title Screen Background", server_disabled)) {
        // Switch Background
        patch_vtable(StartMenuScreen_renderBackground, StartMenuScreen_renderBackground_injection<StartMenuScreen>);
        patch_vtable(Touch_StartMenuScreen_renderBackground, StartMenuScreen_renderBackground_injection<Touch_StartMenuScreen>);
        // Text Color
        patch_address((void *) 0x397ac, (void *) 0xffffffff);
        patch_address((void *) 0x3e10c, (void *) 0xffffffff);
    }

    // Improved Classic Title Screen
    if (feature_has("Improved Classic Title Screen", server_disabled)) {
        // Add Options Button Back To Classic Start Screen
        overwrite_calls(StartMenuScreen_init, StartMenuScreen_init_injection);

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
        patch_address((void *) &Strings::classic_create_button_text, (void *) "Quit");

        // Add Functionality To Quit Button
        overwrite_calls(StartMenuScreen_buttonClicked, StartMenuScreen_buttonClicked_injection);
    }

    // Modern Logo
    const bool modern_logo = feature_has("Use Updated Title", server_disabled);
    if (modern_logo) {
        const char *new_path = "gui/modern_logo.png";
        patch_address((void *) &Strings::title_texture_classic, (void *) new_path);
        patch_address((void *) &Strings::title_texture_touch, (void *) new_path);
    }

    // High-Resolution Title
    if (feature_has("Allow High-Resolution Title", server_disabled) || modern_logo) {
        // Touch
        overwrite_call((void *) 0x3df2c, Textures_getTemporaryTextureData, StartMenuScreen_render_Textures_getTemporaryTextureData_injection);
        overwrite_call((void *) 0x3df98, Mth_min, StartMenuScreen_render_Mth_min_injection);
        // Classic
        overwrite_call((void *) 0x3956c, Textures_getTemporaryTextureData, StartMenuScreen_render_Textures_getTemporaryTextureData_injection);
        overwrite_call((void *) 0x395d8, Mth_min, StartMenuScreen_render_Mth_min_injection);
    }

    // Better Scaling And Position
    bool hijack_version_rendering = false;
    if (feature_has("Improved Classic Title Positioning", server_disabled)) {
        overwrite_call((void *) 0x3956c, Textures_getTemporaryTextureData, StartMenuScreen_render_Textures_getTemporaryTextureData_injection_modern);
        overwrite_call((void *) 0x39528, StartMenuScreen_renderBackground, StartMenuScreen_render_Screen_renderBackground_injection);
        hijack_version_rendering = true;
        adjust_version_y = get_version_y;
    }

    // Add Splashes
    if (feature_has("Add Splashes", server_disabled)) {
        hijack_version_rendering = true;
        _init_splashes();
    }

    // Adjust And Record Version String Rendering
    if (hijack_version_rendering) {
        overwrite_call((void *) 0x39728, GuiComponent_drawString, StartMenuScreen_render_GuiComponent_drawString_injection);
    }

    // Init Welcome Screen
    if (feature_has("Add Welcome Screen", server_disabled)) {
        _init_welcome();
    }
}
