#include <fstream>
#include <cmath>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/compat/compat.h>
#include <mods/touch/touch.h>
#include <mods/title-screen/title-screen.h>

// Improved Title Screen Background
static void StartMenuScreen_render_Screen_renderBackground_injection(Screen *screen) {
    // Draw
    Minecraft *minecraft = screen->minecraft;
    Textures *textures = minecraft->textures;
    std::string texture = "gui/titleBG.png";
    textures->loadAndBindTexture(&texture);
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
    Button *quit_button = &screen->create_button;
    if (button == quit_button) {
        // Quit
        compat_request_exit();
    } else {
        // Call Original Method
        original(screen, button);
    }
}

// Add Splashes
void title_screen_load_splashes(std::vector<std::string> &splashes) {
    std::ifstream stream("data/splashes.txt");
    if (stream.good()) {
        std::string line;
        while (std::getline(stream, line)) {
            if (line.length() > 0) {
                splashes.push_back(line);
            }
        }
        stream.close();
    } else {
        WARN("Unable To Load Splashes");
    }
}
static Screen *last_screen = nullptr;
static std::string current_splash;
static void StartMenuScreen_render_Screen_render_injection(Screen *screen, int x, int y, float param_1) {
    // Call Original Method
    (*Screen_render_vtable_addr)(screen, x, y, param_1);

    // Load Splashes
    static std::vector<std::string> splashes;
    static bool splashes_loaded = false;
    if (!splashes_loaded) {
        // Mark As Loaded
        splashes_loaded = true;
        // Load
        title_screen_load_splashes(splashes);
    }

    // Display Splash
    if (splashes.size() > 0) {
        // Pick Splash
        if (last_screen != screen) {
            last_screen = screen;
            current_splash = splashes[rand() % splashes.size()];
        }
        // Choose Position
        float multiplier = touch_gui ? 0.5f : 1.0f;
        float splash_x = (float(screen->width) / 2.0f) + (94.0f * multiplier);
        float splash_y = 4.0f + (36.0f * multiplier);
        float max_width = 86;
        float max_scale = 2.0f;
        // Draw (From https://github.com/ReMinecraftPE/mcpe/blob/d7a8b6baecf8b3b050538abdbc976f690312aa2d/source/client/gui/screens/StartMenuScreen.cpp#L699-L718)
        glPushMatrix();
        // Position
        glTranslatef(splash_x, splash_y, 0.0f);
        glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);
        // Scale
        int textWidth = screen->font->width(&current_splash);
        float timeMS = float(Common::getTimeMs() % 1000) / 1000.0f;
        float scale = max_scale - Mth::abs(0.1f * Mth::sin(2.0f * float(M_PI) * timeMS));
        float real_text_width = textWidth * max_scale;
        if (real_text_width > max_width) {
            scale *= max_width / real_text_width;
        }
        scale *= multiplier;
        glScalef(scale, scale, scale);
        // Render
        static int line_height = 8;
        screen->drawCenteredString(screen->font, &current_splash, 0, -(float(line_height) / 2), 0xffff00);
        // Finish
        glPopMatrix();
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
        overwrite_virtual_calls(StartMenuScreen_init, StartMenuScreen_init_injection);

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
        overwrite_virtual_calls(StartMenuScreen_buttonClicked, StartMenuScreen_buttonClicked_injection);
    }

    // Add Splashes
    if (feature_has("Add Splashes", server_disabled)) {
        overwrite_call((void *) 0x39764, (void *) StartMenuScreen_render_Screen_render_injection);
        overwrite_call((void *) 0x3e0c4, (void *) StartMenuScreen_render_Screen_render_injection);
        // Init Random
        srand(time(nullptr));
    }
}
