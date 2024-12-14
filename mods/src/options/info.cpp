#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/config.h>
#include <libreborn/util/util.h>

#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/touch/touch.h>
#include <mods/misc/misc.h>
#include <mods/options/info.h>
#include <mods/extend/extend.h>

#include "options-internal.h"

// Button IDs
#define DISCORD_ID 0
#define BACK_ID 1
#define INFO_ID_START 2

// Constants
static constexpr int line_button_padding = 8;
static constexpr int line_button_height = (line_button_padding * 2) + line_height;
static constexpr int padding = 4;
static constexpr int bottom_padding = padding;
static constexpr int inner_padding = padding;
static constexpr int title_padding = 8;
static constexpr int info_text_y_offset = (line_button_height - line_height) / 2;
static constexpr int content_y_offset_top = (title_padding * 2) + line_height;
static constexpr int content_y_offset_bottom = (bottom_padding * 2) + line_button_height;

// Extra Version Info
static std::string extra_version_info =
#ifdef MCPI_IS_APPIMAGE_BUILD
    "AppImage"
#elif defined(MCPI_IS_FLATPAK_BUILD)
    "Flatpak"
#else
    ""
#endif
    ;
static std::string extra_version_info_full = !extra_version_info.empty() ? (" (" + extra_version_info + ")") : "";

// Info Data
struct info_line {
    std::string (*get_text)();
    std::string button_url;
    std::string button_text;
};
std::string info_sound_data_state = "N/A";
static info_line info[] = {
    {
        .get_text = []() {
            return std::string("Version: v") + reborn_get_version() + extra_version_info_full;
        },
        .button_url = MCPI_DOCUMENTATION CHANGELOG_FILE,
        .button_text = "Changelog"
    },
    {
        .get_text = []() {
            return std::string("Profile Directory");
        },
        .button_url = std::string("file://") + home_get(),
        .button_text = "Open"
    },
    {
        .get_text = []() {
            return std::string("Sound Data: ") + info_sound_data_state;
        },
        .button_url = MCPI_DOCUMENTATION "GETTING_STARTED.md#sound",
        .button_text = "More Info"
    },
};
#define info_size int(sizeof(info) / sizeof(info_line))

// Positioned Info
struct info_pos {
    int x;
    int y;
};
struct info_line_position {
    info_pos text;
    info_pos button;
};
static info_line_position positioned_info[info_size];
static int content_height = 0;
static int line_button_width = 0;
static void position_info(Font *font, const int width, const int height) {
    // First Stage (Find Max Text Width)
    int info_text_width = 0;
    for (int i = 0; i < info_size; i++) {
        std::string text = info[i].get_text();
        const int text_width = font->width(text);
        if (text_width > info_text_width) {
            info_text_width = text_width;
        }
    }

    // Second Stage (Initial Positioning)
    int y = 0;
    for (int i = 0; i < info_size; i++) {
        // Padding
        if (i != 0) {
            y += padding;
        }
        // Y
        positioned_info[i].button.y = y;
        positioned_info[i].text.y = y + info_text_y_offset;
        // X
        positioned_info[i].button.x = info_text_width + padding;
        positioned_info[i].text.x = 0;
        // Advance
        y += line_button_height;
    }

    // Third Stage (Find Line Button Width)
    line_button_width = 0;
    for (int i = 0; i < info_size; i++) {
        const int text_width = font->width(info[i].button_text);
        if (text_width > line_button_width) {
            line_button_width = text_width;
        }
    }
    line_button_width += line_button_padding * 2;

    // Fourth Stage (Centering)
    const int info_height = y;
    const int info_width = info_text_width + padding + line_button_width;
    content_height = height - content_y_offset_top - content_y_offset_bottom;
    const int info_y_offset = ((content_height - info_height) / 2) + content_y_offset_top;
    const int info_x_offset = (width - info_width) / 2;
    for (int i = 0; i < info_size; i++) {
        positioned_info[i].button.x += info_x_offset;
        positioned_info[i].button.y += info_y_offset;
        positioned_info[i].text.x += info_x_offset;
        positioned_info[i].text.y += info_y_offset;
    }
}

// Create VTable
struct InfoScreen final : CustomScreen {
    // Buttons
    Button *discord;
    Button *back;
    Button *info_buttons[info_size];
    // Init
    void init() override {
        CustomScreen::init();
        // Info
        for (int i = 0; i < info_size; i++) {
            Button *button = touch_create_button(INFO_ID_START + i, info[i].button_text);
            self->rendered_buttons.push_back(button);
            self->selectable_buttons.push_back(button);
            info_buttons[i] = button;
        }
        // Discord Button
        discord = touch_create_button(DISCORD_ID, "Discord");
        self->rendered_buttons.push_back(discord);
        self->selectable_buttons.push_back(discord);
        // Back Button
        back = touch_create_button(BACK_ID, "Back");
        self->rendered_buttons.push_back(back);
        self->selectable_buttons.push_back(back);
    }
    // Handle Back
    bool handleBackEvent(const bool do_nothing) override {
        if (!do_nothing) {
            OptionsScreen *screen = OptionsScreen::allocate();
            screen->constructor();
            self->minecraft->setScreen((Screen *) screen);
        }
        return true;
    }
    // Rendering
    void render(const int x, const int y, const float param_1) override {
        // Background
        misc_render_background(80, self->minecraft, 0, 0, self->width, self->height);
        misc_render_background(32, self->minecraft, 0, content_y_offset_top, self->width, content_height);
        // Call Original Method
        CustomScreen::render(x, y, param_1);
        // Title
        std::string title = "Reborn Information";
        self->drawCenteredString(self->font, title, self->width / 2, title_padding, 0xffffffff);
        // Info Text
        for (int i = 0; i < info_size; i++) {
            std::string text = info[i].get_text();
            self->drawString(self->font, text, positioned_info[i].text.x, positioned_info[i].text.y, 0xffffffff);
        }
    }
    // Positioning
    void setupPositions() override {
        CustomScreen::setupPositions();
        // Height/Width
        constexpr int width = 120;
        discord->width = back->width = width;
        discord->height = back->height = line_button_height;
        // X/Y
        discord->y = back->y = self->height - bottom_padding - line_button_height;
        discord->x = (self->width / 2) - inner_padding - width;
        back->x = (self->width / 2) + inner_padding;
        // Info
        position_info(self->font, self->width, self->height);
        for (int i = 0; i < info_size; i++) {
            Button *button = info_buttons[i];
            button->width = line_button_width;
            button->height = line_button_height;
            button->x = positioned_info[i].button.x;
            button->y = positioned_info[i].button.y;
        }
    }
    // Cleanup
    ~InfoScreen() override {
        for (Button *button : self->rendered_buttons) {
            button->destructor_deleting();
        }
    }
    // Handle Button Click
    void buttonClicked(Button *button) override {
        if (button->id == BACK_ID) {
            // Back
            self->handleBackEvent(false);
        } else if (button->id == DISCORD_ID) {
            // Open Discord Invite
            open_url(MCPI_DISCORD_INVITE);
        } else if (button->id >= INFO_ID_START) {
            // Open Info URL
            const int i = button->id - INFO_ID_START;
            open_url(info[i].button_url);
        } else {
            CustomScreen::buttonClicked(button);
        }
    }
};

// Create Screen
Screen *_create_options_info_screen() {
    return extend_struct<InfoScreen>();
}
