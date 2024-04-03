// Config Needs To Load First
#include <libreborn/libreborn.h>
#include "game-mode-internal.h"

// Game Mode UI Code Is Useless In Headless Mode
#ifndef MCPI_HEADLESS_MODE

#include <string>
#include <set>

#include <symbols/minecraft.h>

#include <mods/text-input-box/TextInputScreen.h>
#include <mods/touch/touch.h>

// Strings
#define GAME_MODE_STR(mode) ("Game Mode: " mode)
#define SURVIVAL_STR GAME_MODE_STR("Survival")
#define CREATIVE_STR GAME_MODE_STR("Creative")

// Structure
struct CreateWorldScreen {
    TextInputScreen super;
    TextInputBox *name;
    TextInputBox *seed;
    Button *game_mode;
    Button *create;
    Button *back;
};
static void create_world(Minecraft *minecraft, std::string name, bool is_creative, std::string seed);
CUSTOM_VTABLE(create_world_screen, Screen) {
    TextInputScreen::setup(vtable);
    // Constants
    static int line_height = 8;
    static int bottom_padding = 4;
    static int inner_padding = 4;
    static int description_padding = 4;
    static int title_padding = 8;
    // Init
    static Screen_init_t original_init = vtable->init;
    vtable->init = [](Screen *super) {
        original_init(super);
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        // Name
        self->name = TextInputBox::create("World Name", "Unnamed world");
        self->super.m_textInputs->push_back(self->name);
        self->name->init(super->font);
        self->name->setFocused(true);
        // Seed
        self->seed = TextInputBox::create("Seed");
        self->super.m_textInputs->push_back(self->seed);
        self->seed->init(super->font);
        self->seed->setFocused(false);
        // Game Mode
        self->game_mode = touch_create_button(1, CREATIVE_STR);
        super->rendered_buttons.push_back(self->game_mode);
        super->selectable_buttons.push_back(self->game_mode);
        // Create
        self->create = touch_create_button(2, "Create");
        super->rendered_buttons.push_back(self->create);
        super->selectable_buttons.push_back(self->create);
        // Back
        self->back = touch_create_button(3, "Back");
        super->rendered_buttons.push_back(self->back);
        super->selectable_buttons.push_back(self->back);
    };
    // Removal
    static Screen_removed_t original_removed = vtable->removed;
    vtable->removed = [](Screen *super) {
        original_removed(super);
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        delete self->name;
        delete self->seed;
        self->game_mode->vtable->destructor_deleting(self->game_mode);
        self->back->vtable->destructor_deleting(self->back);
        self->create->vtable->destructor_deleting(self->create);
    };
    // Rendering
    static Screen_render_t original_render = vtable->render;
    vtable->render = [](Screen *super, int x, int y, float param_1) {
        // Background
        super->vtable->renderBackground(super);
        // Call Original Method
        original_render(super, x, y, param_1);
        // Title
        std::string title = "Create world";
        Screen_drawCenteredString(super, super->font, &title, super->width / 2, title_padding, 0xffffffff);
        // Game Mode Description
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        bool is_creative = self->game_mode->text == CREATIVE_STR;
        std::string description = is_creative ? Strings_creative_mode_description : Strings_survival_mode_description;
        Screen_drawString(super, super->font, &description, self->game_mode->x, self->game_mode->y + self->game_mode->height + description_padding, 0xa0a0a0);
    };
    // Positioning
    static Screen_setupPositions_t original_setupPositions = vtable->setupPositions;
    vtable->setupPositions = [](Screen *super) {
        original_setupPositions(super);
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        // Height/Width
        int width = 120;
        int height = 24;
        self->create->width = self->back->width = self->game_mode->width = width;
        int seed_width = self->game_mode->width;
        int name_width = width * 1.5f;
        self->create->height = self->back->height = self->game_mode->height = height;
        int text_box_height = self->game_mode->height;
        // Find Center Y
        int top = (title_padding * 2) + line_height;
        int bottom = super->height - self->create->height - (bottom_padding * 2);
        int center_y = ((bottom - top) / 2) + top;
        center_y -= (description_padding + line_height) / 2;
        // X/Y
        self->create->y = self->back->y = super->height - bottom_padding - height;
        self->create->x = self->game_mode->x = (super->width / 2) - inner_padding - width;
        self->back->x = (super->width / 2) + inner_padding;
        int seed_x = self->back->x;
        int name_x = (super->width / 2) - (name_width / 2);
        int name_y = center_y - inner_padding - height;
        self->game_mode->y = center_y + inner_padding;
        int seed_y = self->game_mode->y;
        // Update Text Boxes
        self->name->setSize(name_x, name_y, name_width, text_box_height);
        self->seed->setSize(seed_x, seed_y, seed_width, text_box_height);
    };
    // ESC
    vtable->handleBackEvent = [](Screen *super, bool do_nothing) {
        if (!do_nothing) {
            ScreenChooser_setScreen(&super->minecraft->screen_chooser, 5);
        }
        return true;
    };
    // Button Click
    vtable->buttonClicked = [](Screen *super, Button *button) {
        CreateWorldScreen *self = (CreateWorldScreen *) super;
        bool is_creative = self->game_mode->text == CREATIVE_STR;
        if (button == self->game_mode) {
            // Toggle Game Mode
            self->game_mode->text = is_creative ? SURVIVAL_STR : CREATIVE_STR;
        } else if (button == self->back) {
            // Back
            super->vtable->handleBackEvent(super, false);
        } else if (button == self->create) {
            // Create
            create_world(super->minecraft, self->name->getText(), is_creative, self->seed->getText());
        }
    };
}
static Screen *create_create_world_screen() {
    // Construct
    CreateWorldScreen *screen = new CreateWorldScreen;
    ALLOC_CHECK(screen);
    Screen_constructor(&screen->super.super);

    // Set VTable
    screen->super.super.vtable = get_create_world_screen_vtable();

    // Return
    return (Screen *) screen;
}

// Unique Level Name (https://github.com/ReMinecraftPE/mcpe/blob/d7a8b6baecf8b3b050538abdbc976f690312aa2d/source/client/gui/screens/CreateWorldScreen.cpp#L65-L83)
static std::string getUniqueLevelName(LevelStorageSource *source, const std::string &in) {
    std::set<std::string> maps;
    std::vector<LevelSummary> vls;
    source->vtable->getLevelList(source, &vls);
    for (int i = 0; i < int(vls.size()); i++) {
        const LevelSummary &ls = vls[i];
        maps.insert(ls.folder);
    }
    std::string out = in;
    while (maps.find(out) != maps.end()) {
        out += "-";
    }
    return out;
}

// Create World
static void create_world(Minecraft *minecraft, std::string name, bool is_creative, std::string seed_str) {
    // Get Seed
    int seed;
    seed_str = Util_stringTrim(&seed_str);
    if (!seed_str.empty()) {
        int num;
        if (sscanf(seed_str.c_str(), "%d", &num) > 0) {
            seed = num;
        } else {
            seed = Util_hashCode(&seed_str);
        }
    } else {
        seed = Common_getEpochTimeS();
    }

    // Get Folder Name
    name = Util_stringTrim(&name);
    std::string folder = "";
    for (char c : name) {
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
    folder = getUniqueLevelName(Minecraft_getLevelSource(minecraft), folder);

    // Settings
    LevelSettings settings;
    settings.game_type = is_creative;
    settings.seed = seed;

    // Create World
    minecraft->vtable->selectLevel(minecraft, &folder, &name, &settings);

    // Multiplayer
    Minecraft_hostMultiplayer(minecraft, 19132);

    // Open ProgressScreen
    ProgressScreen *screen = alloc_ProgressScreen();
    ALLOC_CHECK(screen);
    screen = ProgressScreen_constructor(screen);
    Minecraft_setScreen(minecraft, (Screen *) screen);
}

// Redirect Create World Button
#define create_SelectWorldScreen_tick_injection(prefix) \
    static void prefix##SelectWorldScreen_tick_injection(prefix##SelectWorldScreen_tick_t original, prefix##SelectWorldScreen *screen) { \
        if (screen->should_create_world) { \
            /* Open Screen */ \
            Minecraft_setScreen(screen->minecraft, create_create_world_screen()); \
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
    overwrite_virtual_calls(SelectWorldScreen_tick, SelectWorldScreen_tick_injection);
    overwrite_virtual_calls(Touch_SelectWorldScreen_tick, Touch_SelectWorldScreen_tick_injection);
}

#else
void _init_game_mode_ui() {
}
#endif
