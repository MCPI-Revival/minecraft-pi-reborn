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

// Constants
static constexpr int bottom_padding = 4;
static constexpr int inner_padding = 4;
static constexpr int description_padding = 4;
static constexpr int title_padding = 8;
static constexpr int button_height = 24;
static constexpr int content_y_offset_top = (title_padding * 2) + line_height;
static constexpr int content_y_offset_bottom = button_height + (bottom_padding * 2);

// Structure
static void create_world(Minecraft *, std::string, bool, const std::string &);
struct CreateWorldScreen final : TextInputScreen {
    TextInputBox *name;
    TextInputBox *seed;
    Button *game_mode;
    Button *create;
    Button *back;
    // Init
    void init() override {
        TextInputScreen::init();
        // Name
        name = new TextInputBox("World Name", "Unnamed world");
        m_textInputs->push_back(name);
        name->init(self->font);
        name->setFocused(true);
        // Seed
        seed = new TextInputBox("Seed");
        m_textInputs->push_back(seed);
        seed->init(self->font);
        seed->setFocused(false);
        // Game Mode
        game_mode = touch_create_button(1, CREATIVE_STR);
        self->rendered_buttons.push_back(game_mode);
        self->selectable_buttons.push_back(game_mode);
        // Create
        create = touch_create_button(2, "Create");
        self->rendered_buttons.push_back(create);
        self->selectable_buttons.push_back(create);
        // Back
        back = touch_create_button(3, "Back");
        self->rendered_buttons.push_back(back);
        self->selectable_buttons.push_back(back);
    }
    // Removal
    ~CreateWorldScreen() override {
        delete name;
        delete seed;
        game_mode->destructor_deleting();
        back->destructor_deleting();
        create->destructor_deleting();
    }
    // Rendering
    void render(const int x, const int y, const float param_1) override {
        // Background
        misc_render_background(80, self->minecraft, 0, 0, self->width, self->height);
        misc_render_background(32, self->minecraft, 0, content_y_offset_top, self->width, self->height - content_y_offset_top - content_y_offset_bottom);
        // Call Original Method
        TextInputScreen::render(x, y, param_1);
        // Title
        std::string title = "Create world";
        self->drawCenteredString(self->font, title, self->width / 2, title_padding, 0xffffffff);
        // Game Mode Description
        const bool is_creative = game_mode->text == CREATIVE_STR;
        std::string description = is_creative ? Strings::creative_mode_description : Strings::survival_mode_description;
        self->drawString(self->font, description, game_mode->x, game_mode->y + game_mode->height + description_padding, 0xa0a0a0);
    }
    // Positioning
    void setupPositions() override {
        TextInputScreen::setupPositions();
        // Height/Width
        constexpr int width = 120;
        constexpr int height = button_height;
        create->width = back->width = game_mode->width = width;
        const int seed_width = game_mode->width;
        constexpr int name_width = width * 1.5f;
        create->height = back->height = game_mode->height = height;
        const int text_box_height = game_mode->height;
        // Find Center Y
        constexpr int top = content_y_offset_top;
        const int bottom = self->height - content_y_offset_bottom;
        int center_y = ((bottom - top) / 2) + top;
        center_y -= (description_padding + line_height) / 2;
        // X/Y
        create->y = back->y = self->height - bottom_padding - height;
        create->x = game_mode->x = (self->width / 2) - inner_padding - width;
        back->x = (self->width / 2) + inner_padding;
        const int seed_x = back->x;
        const int name_x = (self->width / 2) - (name_width / 2);
        const int name_y = center_y - inner_padding - height;
        game_mode->y = center_y + inner_padding;
        const int seed_y = game_mode->y;
        // Update Text Boxes
        name->setSize(name_x, name_y, name_width, text_box_height);
        seed->setSize(seed_x, seed_y, seed_width, text_box_height);
    }
    // ESC
    bool handleBackEvent(const bool do_nothing) override {
        if (!do_nothing) {
            self->minecraft->screen_chooser.setScreen(5);
        }
        return true;
    }
    // Button Click
    void buttonClicked(Button *button) override {
        const bool is_creative = game_mode->text == CREATIVE_STR;
        if (button == game_mode) {
            // Toggle Game Mode
            game_mode->text = is_creative ? SURVIVAL_STR : CREATIVE_STR;
        } else if (button == back) {
            // Back
            self->handleBackEvent(false);
        } else if (button == create) {
            // Create
            create_world(self->minecraft, name->getText(), is_creative, seed->getText());
        } else {
            TextInputScreen::buttonClicked(button);
        }
    }
};
static Screen *create_create_world_screen() {
    return extend_struct<Screen, CreateWorldScreen>();
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
static void create_world(Minecraft *minecraft, std::string name, const bool is_creative, const std::string &seed_str) {
    // Get Seed
    const int seed = get_seed_from_string(seed_str);

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
