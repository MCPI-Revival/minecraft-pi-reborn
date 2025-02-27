#include <vector>

#include <libreborn/patch.h>
#include <libreborn/util/util.h>
#include <libreborn/util/string.h>

#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/misc/misc.h>
#include "internal.h"

// Callbacks
template <typename... Args>
struct Callbacks {
    std::vector<std::function<void(Args...)>> functions;
    void run(Args... args) {
        for (const std::function<void(Args...)> &func : functions) {
            func(std::forward<Args>(args)...);
        }
    }
};

// Callbacks
#define SETUP_CALLBACK(name, ...) \
    static Callbacks<__VA_ARGS__> &get_misc_##name##_functions() { \
        static Callbacks<__VA_ARGS__> functions; \
        return functions; \
    } \
    void misc_run_on_##name(const std::function<void(__VA_ARGS__)> &function) { \
        get_misc_##name##_functions().functions.push_back(function); \
    }

// Run Functions On Creative Inventory Setup
SETUP_CALLBACK(creative_inventory_setup, FillingContainer *);
// Handle Custom Creative Inventory Setup Behavior
static int Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container, ItemInstance *item_instance) {
    // Call Original Method
    const int ret = filling_container->addItem(item_instance);

    // Run Functions
    get_misc_creative_inventory_setup_functions().run(filling_container);

    // Return
    return ret;
}

// Track Frames
SETUP_CALLBACK(swap_buffers);
HOOK(media_swap_buffers, void, ()) {
    get_misc_swap_buffers_functions().run();
    real_media_swap_buffers()();
}

// API
void misc_run_on_init(const std::function<void(Minecraft *)> &func) {
    overwrite_calls(Minecraft_init, [func](Minecraft_init_t original, Minecraft *self) {
        original(self);
        func(self);
    });
}
void misc_run_on_update(const std::function<void(Minecraft *)> &func) {
    overwrite_calls(Minecraft_update, [func](Minecraft_update_t original, Minecraft *self) {
        original(self);
        func(self);
    });
}
void misc_run_on_tick(const std::function<void(Minecraft *)> &func) {
    overwrite_calls(Minecraft_tick, [func](Minecraft_tick_t original, Minecraft *self, const int tick, const int max_ticks) {
        original(self, tick, max_ticks);
        func(self);
    });
}
void misc_run_on_recipes_setup(const std::function<void(Recipes *)> &func) {
    overwrite_calls(Recipes_constructor, [func](Recipes_constructor_t original, Recipes *self) {
        original(self);
        func(self);
        return self;
    });
}
void misc_run_on_furnace_recipes_setup(const std::function<void(FurnaceRecipes *)> &func) {
    overwrite_calls(FurnaceRecipes_constructor, [func](FurnaceRecipes_constructor_t original, FurnaceRecipes *self) {
        original(self);
        func(self);
        return self;
    });
}
void misc_run_on_tiles_setup(const std::function<void()> &func) {
    overwrite_calls(Tile_initTiles, [func](Tile_initTiles_t original) {
        func();
        original();
    });
}
void misc_run_on_items_setup(const std::function<void()> &func) {
    overwrite_calls(Item_initItems, [func](Item_initItems_t original) {
        original();
        func();
    });
}
void misc_run_on_language_setup(const std::function<void()> &func) {
    overwrite_calls(I18n_loadLanguage, [func](I18n_loadLanguage_t original, AppPlatform *self, std::string language_name) {
        original(self, language_name);
        func();
    });
}
void misc_run_on_game_key_press(const std::function<bool(Minecraft *, int)> &func) {
    overwrite_calls(Gui_handleKeyPressed, [func](Gui_handleKeyPressed_t original, Gui *self, const int key) {
        if (func(self->minecraft, key)) {
            return;
        }
        original(self, key);
    });
}
void misc_run_on_key_press(const std::function<bool(Minecraft *, int)> &func) {
    misc_run_on_game_key_press(func);
    overwrite_calls(Screen_keyPressed, [func](Screen_keyPressed_t original, Screen *self, const int key) {
        if (func(self->minecraft, key)) {
            return;
        }
        original(self, key);
    });
}

// Render Fancy Background
void misc_render_background(int color, const Minecraft *minecraft, const int x, const int y, const int width, const int height) {
    // https://github.com/ReMinecraftPE/mcpe/blob/f0d65eaecec1b3fe9c2f2b251e114a890c54ab77/source/client/gui/components/RolledSelectionList.cpp#L169-L179
    media_glColor4f(1, 1, 1, 1);
    minecraft->textures->loadAndBindTexture("gui/background.png");
    Tesselator *t = &Tesselator::instance;
    t->begin(GL_QUADS);
    t->color(color, color, color, 255);
    float x1 = float(x);
    float x2 = x1 + float(width);
    float y1 = float(y);
    float y2 = y1 + float(height);
    t->vertexUV(x1, y2, 0.0f, x1 / 32.0f, y2 / 32.0f);
    t->vertexUV(x2, y2, 0.0f, x2 / 32.0f, y2 / 32.0f);
    t->vertexUV(x2, y1, 0.0f, x2 / 32.0f, y1 / 32.0f);
    t->vertexUV(x1, y1, 0.0f, x1 / 32.0f, y1 / 32.0f);
    t->draw();
}

// Entity Names
static std::pair<std::string, std::string> format_entity_name(const std::string &friendly_name) {
    std::string api_name = friendly_name;
    for (char &c : api_name) {
        c = c == ' ' ? '_' : std::toupper(c);
    }
    return {friendly_name, api_name};
}
std::pair<std::string, std::string> misc_get_entity_type_name(Entity *entity) {
    if (entity) {
        if (entity->isPlayer()) {
            // Player
            return format_entity_name("Player");
        } else {
            const EntityType type = static_cast<EntityType>(entity->getEntityTypeId());
            if (misc_get_entity_type_names().contains(type)) {
                // Normal Entity
                return misc_get_entity_type_names()[type];
            } else if (type == EntityType::UNKNOWN) {
                // Special Entity
                static std::unordered_map<void *, std::string> vtable_to_name = {
                    {(void *) Particle_vtable::base, "Particle"},
                    {(void *) TripodCamera_vtable::base, "Tripod Camera"},
                    {(void *) CameraEntity_vtable::base, "API Camera"}
                };
                void *vtable = entity->vtable;
                if (vtable_to_name.contains(vtable)) {
                    return format_entity_name(vtable_to_name[vtable]);
                }
            }
        }
    }
    // Invalid
    return format_entity_name("Unknown");
}
std::string misc_get_entity_name(Entity *entity) {
    if (entity && entity->isPlayer()) {
        return ((Player *) entity)->username;
    } else {
        return misc_get_entity_type_name(entity).first;
    }
}
std::map<EntityType, std::pair<std::string, std::string>> &misc_get_entity_type_names() {
    static std::map<EntityType, std::pair<std::string, std::string>> names = {
        {EntityType::CHICKEN, format_entity_name("Chicken")},
        {EntityType::COW, format_entity_name("Cow")},
        {EntityType::PIG, format_entity_name("Pig")},
        {EntityType::SHEEP, format_entity_name("Sheep")},
        {EntityType::ZOMBIE, format_entity_name("Zombie")},
        {EntityType::CREEPER, format_entity_name("Creeper")},
        {EntityType::SKELETON, format_entity_name("Skeleton")},
        {EntityType::SPIDER, format_entity_name("Spider")},
        {EntityType::ZOMBIE_PIGMAN, {"Zombie Pigman", "PIG_ZOMBIE"}},
        {EntityType::DROPPED_ITEM, format_entity_name("Dropped Item")},
        {EntityType::PRIMED_TNT, format_entity_name("Primed TNT")},
        {EntityType::FALLING_SAND, format_entity_name("Falling Block")},
        {EntityType::PAINTING, format_entity_name("Painting")},
        {EntityType::ARROW, format_entity_name("Arrow")},
        {EntityType::THROWN_SNOWBALL, format_entity_name("Snowball")},
        {EntityType::THROWN_EGG, format_entity_name("Egg")}
    };
    return names;
}

// Spawn Entities
Entity *misc_make_entity_from_id(Level *level, const int id) {
    if (id < static_cast<int>(EntityType::DROPPED_ITEM)) {
        // Spwn Mob
        return (Entity *) MobFactory::CreateMob(id, level);
    } else {
        // Spawn Entity
        Entity *entity = EntityFactory::CreateEntity(id, level);
        switch (id) {
            case static_cast<int>(EntityType::PAINTING): {
                // Fix Crash
                ((Painting *) entity)->motive = Motive::DefaultImage;
                break;
            }
            case static_cast<int>(EntityType::FALLING_SAND): {
                // Sensible Default
                FallingTile *sand = (FallingTile *) entity;
                sand->tile_id = Tile::sand->id;
                sand->time = 1;
                break;
            }
            case static_cast<int>(EntityType::DROPPED_ITEM): {
                // Sensible Default
                ((ItemEntity *) entity)->item.constructor_item(Item::sword_iron);
                break;
            }
        }
        return entity;
    }
}

// Username In Unicode
std::string misc_get_player_username_utf(const Player *player) {
    return from_cp437(player->username);
}

// Init
void _init_misc_api() {
    // Handle Custom Creative Inventory Setup Behavior
    overwrite_call((void *) 0x8e0fc, FillingContainer_addItem, Inventory_setupDefault_FillingContainer_addItem_call_injection);
}
