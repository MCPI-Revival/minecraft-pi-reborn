#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/touch/touch.h>

#include "options-internal.h"

// Button IDs
#define DISCORD_ID 0
#define BACK_ID 1
#define INFO_ID_START 2

// Constants
static int line_button_width = 80;
static int line_button_height = 24;
static int padding = 4;
static int line_height = 8;
static int bottom_padding = padding;
static int inner_padding = padding;
static int title_padding = 8;
static int info_text_y_offset = (line_button_height - line_height) / 2;
static int content_y_offset_top = (title_padding * 2) + line_height;
static int content_y_offset_bottom = (bottom_padding * 2) + line_button_height;

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

// Profile Directory
static std::string profile_directory_suffix =
#ifdef MCPI_IS_FLATPAK_BUILD
    "/.var/app/" MCPI_APP_ID "/.minecraft-pi"
#else
    "/.minecraft-pi"
#endif
    ;
static std::string get_profile_directory_url() {
    const char *home = getenv("HOME");
    if (home == nullptr) {
        IMPOSSIBLE();
    }
    return std::string("file://") + home + profile_directory_suffix;
}

// Info Data
#define MORE_INFO_TEXT "More Info"
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
        .button_url = "https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/src/branch/master/docs/CHANGELOG.md",
        .button_text = MORE_INFO_TEXT
    },
    {
        .get_text = []() {
            return std::string("Profile Directory");
        },
        .button_url = get_profile_directory_url(),
        .button_text = "Open"
    },
    {
        .get_text = []() {
            return std::string("Sound Data: ") + info_sound_data_state;
        },
        .button_url = "https://gitea.thebrokenrail.com/minecraft-pi-reborn/minecraft-pi-reborn/src/branch/master/docs/SOUND.md",
        .button_text = MORE_INFO_TEXT
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
static void position_info(Font *font, int width, int height) {
    // First Stage (Find Max Text Width)
    int info_text_width = 0;
    for (int i = 0; i < info_size; i++) {
        std::string text = info[i].get_text();
        int text_width = font->width(&text);
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

    // Third Stage (Centering)
    int info_height = y;
    int info_width = info_text_width + padding + line_button_width;
    content_height = height - content_y_offset_top - content_y_offset_bottom;
    int info_y_offset = ((content_height - info_height) / 2) + content_y_offset_top;
    int info_x_offset = (width - info_width) / 2;
    for (int i = 0; i < info_size; i++) {
        positioned_info[i].button.x += info_x_offset;
        positioned_info[i].button.y += info_y_offset;
        positioned_info[i].text.x += info_x_offset;
        positioned_info[i].text.y += info_y_offset;
    }
}

// Open URL
static void open_url(const std::string &url) {
    int return_code;
    const char *command[] = {"xdg-open", url.c_str(), nullptr};
    char *output = run_command(command, &return_code, nullptr);
    if (output != nullptr) {
        free(output);
    }
    if (!is_exit_status_success(return_code)) {
        WARN("Unable To Open URL: %s", url.c_str());
    }
}

// Render Fancy Background
static void render_background(Minecraft *minecraft, int x, int y, int width, int height) {
    // https://github.com/ReMinecraftPE/mcpe/blob/f0d65eaecec1b3fe9c2f2b251e114a890c54ab77/source/client/gui/components/RolledSelectionList.cpp#L169-L179
    std::string texture = "gui/background.png";
    minecraft->textures->loadAndBindTexture(&texture);
    Tesselator *t = &Tesselator_instance;
    t->begin(7);
    t->color(32, 32, 32, 255);
    float x1 = x;
    float x2 = x + width;
    float y1 = y;
    float y2 = y + height;
    t->vertexUV(x1, y2, 0.0f, x1 / 32.0f, y2 / 32.0f);
    t->vertexUV(x2, y2, 0.0f, x2 / 32.0f, y2 / 32.0f);
    t->vertexUV(x2, y1, 0.0f, x2 / 32.0f, y1 / 32.0f);
    t->vertexUV(x1, y1, 0.0f, x1 / 32.0f, y1 / 32.0f);
    t->draw();
}

// Create VTable
CUSTOM_VTABLE(info_screen, Screen) {
    // Buttons
    static Button *discord;
    static Button *back;
    static Button *info_buttons[info_size];
    // Init
    vtable->init = [](Screen *self) {
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
    };
    // Handle Back
    vtable->handleBackEvent = [](Screen *self, bool do_nothing) {
        if (!do_nothing) {
            OptionsScreen *screen = alloc_OptionsScreen();
            ALLOC_CHECK(screen);
            screen->constructor();
            self->minecraft->setScreen((Screen *) screen);
        }
        return true;
    };
    // Rendering
    static Screen_render_t original_render = vtable->render;
    vtable->render = [](Screen *self, int x, int y, float param_1) {
        // Background
        self->vtable->renderBackground(self);
        // Gradient
        render_background(self->minecraft, 0, content_y_offset_top, self->width, content_height);
        // Call Original Method
        original_render(self, x, y, param_1);
        // Title
        std::string title = "Reborn Information";
        self->drawCenteredString(self->font, &title, self->width / 2, title_padding, 0xffffffff);
        // Info Text
        for (int i = 0; i < info_size; i++) {
            std::string text = info[i].get_text();
            self->drawString(self->font, &text, positioned_info[i].text.x, positioned_info[i].text.y, 0xffffffff);
        }
    };
    // Positioning
    vtable->setupPositions = [](Screen *self) {
        // Height/Width
        int width = 120;
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
    };
    // Cleanup
    vtable->removed = [](Screen *self) {
        for (Button *button : self->rendered_buttons) {
            button->destructor_deleting();
        }
    };
    // Handle Button Click
    vtable->buttonClicked = [](Screen *self, Button *button) {
        if (button->id == BACK_ID) {
            // Back
            self->handleBackEvent(false);
        } else if (button->id == DISCORD_ID) {
            // Open Discord Invite
            open_url("https://discord.gg/mcpi-revival-740287937727561779");
        } else if (button->id >= INFO_ID_START) {
            // Open Info URL
            int i = button->id - INFO_ID_START;
            open_url(info[i].button_url);
        }
    };
}

// Create Screen
Screen *_create_options_info_screen() {
    // Allocate
    Screen *screen = alloc_Screen();
    ALLOC_CHECK(screen);
    screen->constructor();

    // Set VTable
    screen->vtable = get_info_screen_vtable();

    // Return
    return screen;
}