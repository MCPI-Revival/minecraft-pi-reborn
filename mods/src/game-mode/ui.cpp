#include <string>
#include <set>
#include <utility>

#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>

#include <mods/text-input-box/TextInputScreen.h>
#include <mods/touch/touch.h>
#include <mods/misc/misc.h>
#include <mods/game-mode/game-mode.h>
#include "game-mode-internal.h"

// Strings
#define GAME_MODE_STR(mode) ("Game Mode: " mode)
#define SURVIVAL_STR GAME_MODE_STR("Survival")
#define CREATIVE_STR GAME_MODE_STR("Creative")

// Structure
EXTEND_STRUCT(CreateWorldScreen, Screen, struct {
    TextInputScreen text_input;
    TextInputBox *name;
    TextInputBox *seed;
    Button *game_mode;
    Button *create;
    Button *back;
});
static void create_world(Minecraft *minecraft, std::string name, bool is_creative, std::string seed_str);
CUSTOM_VTABLE(create_world_screen, Screen) {
    TextInputScreen::setup<CreateWorldScreen>(vtable);
    // Constants
    static constexpr int bottom_padding = 4;
    static constexpr int inner_padding = 4;
    static constexpr int description_padding = 4;
    static constexpr int title_padding = 8;
    static constexpr int button_height = 24;
    static constexpr int content_y_offset_top = (title_padding * 2) + line_height;
    static constexpr int content_y_offset_bottom = button_height + (bottom_padding * 2);
    // Init
    static Screen_init_t original_init = vtable->init;
    vtable->init = [](Screen *super) {
        original_init(super);
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        // Name
        self->data.name = new TextInputBox("World Name", "Unnamed world");
        self->data.text_input.m_textInputs->push_back(self->data.name);
        self->data.name->init(super->font);
        self->data.name->setFocused(true);
        // Seed
        self->data.seed = new TextInputBox("Seed");
        self->data.text_input.m_textInputs->push_back(self->data.seed);
        self->data.seed->init(super->font);
        self->data.seed->setFocused(false);
        // Game Mode
        self->data.game_mode = touch_create_button(1, CREATIVE_STR);
        super->rendered_buttons.push_back(self->data.game_mode);
        super->selectable_buttons.push_back(self->data.game_mode);
        // Create
        self->data.create = touch_create_button(2, "Create");
        super->rendered_buttons.push_back(self->data.create);
        super->selectable_buttons.push_back(self->data.create);
        // Back
        self->data.back = touch_create_button(3, "Back");
        super->rendered_buttons.push_back(self->data.back);
        super->selectable_buttons.push_back(self->data.back);
    };
    // Removal
    static Screen_removed_t original_removed = vtable->removed;
    vtable->removed = [](Screen *super) {
        original_removed(super);
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        delete self->data.name;
        delete self->data.seed;
        self->data.game_mode->destructor_deleting();
        self->data.back->destructor_deleting();
        self->data.create->destructor_deleting();
    };
    // Rendering
    static Screen_render_t original_render = vtable->render;
    vtable->render = [](Screen *super, const int x, const int y, const float param_1) {
        // Background
        misc_render_background(80, super->minecraft, 0, 0, super->width, super->height);
        misc_render_background(32, super->minecraft, 0, content_y_offset_top, super->width, super->height - content_y_offset_top - content_y_offset_bottom);
        // Call Original Method
        original_render(super, x, y, param_1);
        // Title
        std::string title = "Create world";
        super->drawCenteredString(super->font, title, super->width / 2, title_padding, 0xffffffff);
        // Game Mode Description
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        const bool is_creative = self->data.game_mode->text == CREATIVE_STR;
        std::string description = is_creative ? Strings::creative_mode_description : Strings::survival_mode_description;
        super->drawString(super->font, description, self->data.game_mode->x, self->data.game_mode->y + self->data.game_mode->height + description_padding, 0xa0a0a0);
    };
    // Positioning
    static Screen_setupPositions_t original_setupPositions = vtable->setupPositions;
    vtable->setupPositions = [](Screen *super) {
        original_setupPositions(super);
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        // Height/Width
        constexpr int width = 120;
        const int height = button_height;
        self->data.create->width = self->data.back->width = self->data.game_mode->width = width;
        int seed_width = self->data.game_mode->width;
        int name_width = width * 1.5f;
        self->data.create->height = self->data.back->height = self->data.game_mode->height = height;
        int text_box_height = self->data.game_mode->height;
        // Find Center Y
        const int top = content_y_offset_top;
        const int bottom = super->height - content_y_offset_bottom;
        int center_y = ((bottom - top) / 2) + top;
        center_y -= (description_padding + line_height) / 2;
        // X/Y
        self->data.create->y = self->data.back->y = super->height - bottom_padding - height;
        self->data.create->x = self->data.game_mode->x = (super->width / 2) - inner_padding - width;
        self->data.back->x = (super->width / 2) + inner_padding;
        const int seed_x = self->data.back->x;
        const int name_x = (super->width / 2) - (name_width / 2);
        const int name_y = center_y - inner_padding - height;
        self->data.game_mode->y = center_y + inner_padding;
        const int seed_y = self->data.game_mode->y;
        // Update Text Boxes
        self->data.name->setSize(name_x, name_y, name_width, text_box_height);
        self->data.seed->setSize(seed_x, seed_y, seed_width, text_box_height);
    };
    // ESC
    vtable->handleBackEvent = [](Screen *super, const bool do_nothing) {
        if (!do_nothing) {
            super->minecraft->screen_chooser.setScreen(5);
        }
        return true;
    };
    // Button Click
    vtable->buttonClicked = [](Screen *super, Button *button) {
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        const bool is_creative = self->data.game_mode->text == CREATIVE_STR;
        if (button == self->data.game_mode) {
            // Toggle Game Mode
            self->data.game_mode->text = is_creative ? SURVIVAL_STR : CREATIVE_STR;
        } else if (button == self->data.back) {
            // Back
            super->handleBackEvent(false);
        } else if (button == self->data.create) {
            // Create
            create_world(super->minecraft, self->data.name->getText(), is_creative, self->data.seed->getText());
        }
    };
}
static Screen *create_create_world_screen() {
    // Construct
    CreateWorldScreen *screen = new CreateWorldScreen;
    ALLOC_CHECK(screen);
    screen->super()->constructor();

    // Set VTable
    screen->super()->vtable = get_create_world_screen_vtable();

    // Return
    return (Screen *) screen;
}

// Unique Level Name (https://github.com/ReMinecraftPE/mcpe/blob/d7a8b6baecf8b3b050538abdbc976f690312aa2d/source/client/gui/screens/CreateWorldScreen.cpp#L65-L83)
static std::string getUniqueLevelName(LevelStorageSource *source, const std::string &in) {
    std::set<std::string> maps;
    std::vector<LevelSummary> vls;
    source->getLevelList(vls);
    for (int i = 0; i < int(vls.size()); i++) {
        const LevelSummary &ls = vls[i];
        maps.insert(ls.folder);
    }
    std::string out = in;
    while (maps.contains(out)) {
        out += "-";
    }
    return out;
}

// Create World
int get_seed_from_string(std::string str) {
    int seed;
    str = Util::stringTrim(str);
    if (!str.empty()) {
        int num;
        if (sscanf(str.c_str(), "%d", &num) > 0) {
            seed = num;
        } else {
            seed = Util::hashCode(str);
        }
    } else {
        seed = Common::getEpochTimeS();
    }
    return seed;
}
static void create_world(Minecraft *minecraft, std::string name, const bool is_creative, std::string seed_str) {
    // Get Seed
    const int seed = get_seed_from_string(std::move(seed_str));

    // Get Folder Name
    name = Util::stringTrim(name);
    std::string folder = "";
    for (const char c : name) {
        if (
            c >= ' ' && c <= '~' &&
            c != '/' &&
            c != '\\' &&
            c != '`' &&
            c != '?' &&
            c != '*' &&
            c != '<' &&
            c != '>' &&
            c != '|' &&
            c != '"' &&
            c != ':'
        ) {
            folder += c;
        }
    }
    if (folder.empty()) {
        folder = "World";
    }
    folder = getUniqueLevelName(minecraft->getLevelSource(), folder);

    // Settings
    LevelSettings settings;
    settings.game_type = is_creative;
    settings.seed = seed;

    // Create World
    minecraft->selectLevel(folder, name, settings);

    // Multiplayer
    minecraft->hostMultiplayer(19132);

    // Open ProgressScreen
    ProgressScreen *screen = ProgressScreen::allocate();
    ALLOC_CHECK(screen);
    screen = screen->constructor();
    minecraft->setScreen((Screen *) screen);
}

// Redirect Create World Button
#define create_SelectWorldScreen_tick_injection(prefix) \
    static void prefix##SelectWorldScreen_tick_injection(prefix##SelectWorldScreen_tick_t original, prefix##SelectWorldScreen *screen) { \
        if (screen->should_create_world) { \
            /* Open Screen */ \
            screen->minecraft->setScreen(create_create_world_screen()); \
            /* Finish */ \
            screen->should_create_world = false; \
        } else { \
            /* Call Original Method */ \
            original(screen); \
        } \
    }
create_SelectWorldScreen_tick_injection()
create_SelectWorldScreen_tick_injection(Touch_)

// Init
void _init_game_mode_ui() {
    // Hijack Create World Button
    overwrite_calls(SelectWorldScreen_tick, SelectWorldScreen_tick_injection);
    overwrite_calls(Touch_SelectWorldScreen_tick, Touch_SelectWorldScreen_tick_injection);
}
