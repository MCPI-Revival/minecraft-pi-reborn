#include <utility>
#include <vector>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#ifndef MCPI_HEADLESS_MODE
#include <GLES/gl.h>
#endif

#include <mods/misc/misc.h>
#include "misc-internal.h"

// Callbacks
#define STORE_CALLBACK(name, type) \
    static std::vector<misc_update_function_##type##_t> &get_misc_##name##_functions() { \
        static std::vector<misc_update_function_##type##_t> functions; \
        return functions; \
    } \
    void misc_run_on_##name(misc_update_function_##type##_t function) { \
        get_misc_##name##_functions().push_back(function); \
    }
#define SETUP_CALLBACK(name, type) \
    STORE_CALLBACK(name, type) \
    static void handle_misc_##name(type *obj) { \
        for (misc_update_function_##type##_t function : get_misc_##name##_functions()) { \
            function(obj); \
        } \
    }

// Run Functions On Creative Inventory Setup
SETUP_CALLBACK(creative_inventory_setup, FillingContainer);
// Handle Custom Creative Inventory Setup Behavior
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container, ItemInstance *item_instance) {
    // Call Original Method
    filling_container->addItem(item_instance);

    // Run Functions
    handle_misc_creative_inventory_setup(filling_container);
}

// Render Fancy Background
void misc_render_background(int color, Minecraft *minecraft, int x, int y, int width, int height) {
    // https://github.com/ReMinecraftPE/mcpe/blob/f0d65eaecec1b3fe9c2f2b251e114a890c54ab77/source/client/gui/components/RolledSelectionList.cpp#L169-L179
#ifndef MCPI_HEADLESS_MODE
    glColor4f(1, 1, 1, 1);
#endif
    std::string texture = "gui/background.png";
    minecraft->textures->loadAndBindTexture(&texture);
    Tesselator *t = &Tesselator::instance;
    t->begin(7);
    t->color(color, color, color, 255);
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

// Init
void _init_misc_api() {
    // Handle Custom Creative Inventory Setup Behavior
    overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
}
