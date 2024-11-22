#include <fstream>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/touch/touch.h>
#include <mods/options/info.h>
#include <mods/misc/misc.h>
#include <mods/extend/extend.h>

#include "title-screen-internal.h"

// Constants
static std::string line1 = "Welcome to " MCPI_APP_TITLE " v" MCPI_VERSION "!";
static constexpr int button_width = 120;
static constexpr int button_height = 24;
static constexpr int line_padding = 28;
static constexpr int button_padding = 4;

// Track Whether To Show Screen
static std::string get_tracker_file() {
    return std::string(home_get()) + "/.welcome-tracker";
}
static bool should_show_welcome() {
    // Open File
    std::ifstream stream(get_tracker_file());
    if (!stream) {
        return true;
    }
    // Read Line
    std::string line;
    std::getline(stream, line);
    const bool invalid = line != MCPI_VERSION;
    // Close File
    stream.close();
    // Return
    return invalid;
}
static void mark_welcome_as_shown() {
    // Open File
    std::ofstream stream(get_tracker_file());
    if (!stream) {
        return;
    }
    // Write
    stream << MCPI_VERSION << std::endl;
    // Close File
    stream.close();
}

// Position GUI
static Button *getting_started;
static Button *changelog;
static Button *proceed;
static int text_y;
static void position_screen(const int width, const int height) {
    // Width/Height
    getting_started->width = changelog->width = proceed->width = button_width;
    getting_started->height = changelog->height = proceed->height = button_height;
    // X
    proceed->x = (width / 2) - (button_width / 2);
    getting_started->x = (width / 2) - button_padding - button_width;
    changelog->x = (width / 2) + button_padding;
    // Y
    text_y = 0;
    getting_started->y = changelog->y = line_height + line_padding;
    proceed->y = getting_started->y + button_height + (button_padding * 2);
    // Center
    const int content_height = proceed->y + proceed->height;
    const int y_offset = (height - content_height) / 2;
    text_y += y_offset;
    getting_started->y += y_offset;
    changelog->y += y_offset;
    proceed->y += y_offset;
}

// Welcome Screen
struct WelcomeScreen final : CustomScreen {
    // Init
    void init() override {
        CustomScreen::init();
        // Buttons
        getting_started = touch_create_button(0, "Getting Started");
        changelog = touch_create_button(1, "Changelog");
        proceed = touch_create_button(2, "Proceed");
        for (Button *button : {getting_started, changelog, proceed}) {
            self->rendered_buttons.push_back(button);
            self->selectable_buttons.push_back(button);
        }
    }
    // Rendering
    void render(const int x, const int y, const float param_1) override {
        // Background
        self->renderBackground();
        // Call Original Method
        CustomScreen::render(x, y, param_1);
        // Text
        self->drawCenteredString(self->font, line1, self->width / 2, text_y, 0xFFFFFFFF);
    }
    // Positioning
    void setupPositions() override {
        CustomScreen::setupPositions();
        position_screen(self->width, self->height);
    }
    // Cleanup
    ~WelcomeScreen() override {
        for (Button *button : self->rendered_buttons) {
            button->destructor_deleting();
        }
    }
    // Handle Button Click
    void buttonClicked(Button *button) override {
        if (button == getting_started) {
            open_url(MCPI_DOCUMENTATION "GETTING_STARTED.md");
        } else if (button == changelog) {
            open_url(MCPI_DOCUMENTATION CHANGELOG_FILE);
        } else if (button == proceed) {
            mark_welcome_as_shown();
            self->minecraft->screen_chooser.setScreen(1);
        } else {
            CustomScreen::buttonClicked(button);
        }
    }
};
static Screen *create_welcome_screen() {
    return extend_struct<WelcomeScreen>();
}

// Show Welcome Screen
static void NinecraftApp_init_ScreenChooser_setScreen_injection(ScreenChooser *self, int id) {
    if (should_show_welcome()) {
        // Show Welcome Screen
        self->minecraft->setScreen(create_welcome_screen());
    } else {
        // Show Start Screen
        self->setScreen(id);
    }
}

// Init
void _init_welcome() {
    // Hijack Start Screen
    overwrite_call((void *) 0x14a34, (void *) NinecraftApp_init_ScreenChooser_setScreen_injection);
}