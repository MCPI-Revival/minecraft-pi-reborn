#include <string>
#include <fstream>
#include <streambuf>
#include <algorithm>

#include <cstring>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/feature/feature.h>
#include "misc-internal.h"
#include <mods/misc/misc.h>

// Read Asset File
static AppPlatform_readAssetFile_return_value AppPlatform_readAssetFile_injection(__attribute__((unused)) AppPlatform *app_platform, std::string const& path) {
    // Open File
    std::ifstream stream("data/" + path, std::ios_base::binary | std::ios_base::ate);
    if (!stream) {
        // Does Not Exist
        AppPlatform_readAssetFile_return_value ret;
        ret.length = -1;
        ret.data = NULL;
        return ret;
    }
    // Read File
    long len = stream.tellg();
    char *buf = new char[len];
    stream.seekg(0, stream.beg);
    stream.read(buf, len);
    stream.close();
    // Return String
    AppPlatform_readAssetFile_return_value ret;
    ret.length = len;
    ret.data = strdup(buf);
    return ret;
}

// Add Missing Buttons To Pause Menu
static void PauseScreen_init_injection(PauseScreen *screen) {
    // Call Original Method
    PauseScreen_init_non_virtual(screen);

    // Check If Server
    Minecraft *minecraft = screen->minecraft;
    RakNetInstance *rak_net_instance = minecraft->rak_net_instance;
    if (rak_net_instance != NULL) {
        if (rak_net_instance->vtable->isServer(rak_net_instance)) {
            // Add Button
            std::vector<Button *> *rendered_buttons = &screen->rendered_buttons;
            std::vector<Button *> *selectable_buttons = &screen->selectable_buttons;
            Button *button = screen->server_visibility_button;
            rendered_buttons->push_back(button);
            selectable_buttons->push_back(button);

            // Update Button Text
            PauseScreen_updateServerVisibilityText(screen);
        }
    }
}

// Implement crafting remainders
void PaneCraftingScreen_craftSelectedItem_PaneCraftingScreen_recheckRecipes_injection(PaneCraftingScreen *self) {
    // Check for crafting remainders
    CItem *item = self->item;
    for (size_t i = 0; i < item->ingredients.size(); i++) {
        ItemInstance requested_item_instance = item->ingredients[i].requested_item;
        Item *requested_item = Item_items[requested_item_instance.id];
        ItemInstance *craftingRemainingItem = requested_item->vtable->getCraftingRemainingItem(requested_item, &requested_item_instance);
        if (craftingRemainingItem != NULL) {
            // Add or drop remainder
            LocalPlayer *player = self->minecraft->player;
            if (!player->inventory->vtable->add(player->inventory, craftingRemainingItem)) {
                // Drop
                player->vtable->drop(player, craftingRemainingItem, false);
            }
        }
    }
    // Call Original Method
    PaneCraftingScreen_recheckRecipes(self);
}

ItemInstance *Item_getCraftingRemainingItem_injection(Item *self, ItemInstance *item_instance) {
    if (self->craftingRemainingItem != NULL) {
        ItemInstance *ret = alloc_ItemInstance();
        ret->id = self->craftingRemainingItem->id;
        ret->count = item_instance->count;
        ret->auxiliary = 0;
        return ret;
    }
    return NULL;
}

// Sort Chunks
struct chunk_data {
    Chunk *chunk;
    float distance;
};
#define MAX_CHUNKS_SIZE 24336
static chunk_data data[MAX_CHUNKS_SIZE];
static void sort_chunks(Chunk **chunks_begin, Chunk **chunks_end, DistanceChunkSorter sorter) {
    // Calculate Distances
    int chunks_size = chunks_end - chunks_begin;
    if (chunks_size > MAX_CHUNKS_SIZE) {
        IMPOSSIBLE();
    }
    for (int i = 0; i < chunks_size; i++) {
        Chunk *chunk = chunks_begin[i];
        float distance = Chunk_distanceToSqr(chunk, (Entity *) sorter.mob);
        if ((1024.0 <= distance) && chunk->y < 0x40) {
            distance = distance * 10.0;
        }
        data[i].chunk = chunk;
        data[i].distance = distance;
    }

    // Sort
    std::sort(data, data + chunks_size, [](chunk_data &a, chunk_data &b) {
        return a.distance < b.distance;
    });
    for (int i = 0; i < chunks_size; i++) {
        chunks_begin[i] = data[i].chunk;
    }
}

// Display Date In Select World Screen
static std::string AppPlatform_linux_getDateString_injection(__attribute__((unused)) AppPlatform_linux *app_platform, int time) {
    // From https://github.com/ReMinecraftPE/mcpe/blob/56e51027b1c2e67fe5a0e8a091cefe51d4d11926/platforms/sdl/base/AppPlatform_sdl_base.cpp#L68-L84
    time_t tt = time;
    struct tm t;
    gmtime_r(&tt, &t);
    char buf[2048];
    strftime(buf, sizeof buf, "%b %d %Y %H:%M:%S", &t);
    return std::string(buf);
}

// Init
void _init_misc_cpp() {
    // Implement AppPlatform::readAssetFile So Translations Work
    if (feature_has("Load Language Files", server_enabled)) {
        overwrite((void *) *AppPlatform_readAssetFile_vtable_addr, (void *) AppPlatform_readAssetFile_injection);
    }

    // Fix Pause Menu
    if (feature_has("Fix Pause Menu", server_disabled)) {
        // Add Missing Buttons To Pause Menu
        patch_address(PauseScreen_init_vtable_addr, (void *) PauseScreen_init_injection);
    }

    // Implement Crafting Remainders
    overwrite_call((void *) 0x2e230, (void *) PaneCraftingScreen_craftSelectedItem_PaneCraftingScreen_recheckRecipes_injection);
    overwrite((void *) Item_getCraftingRemainingItem_non_virtual, (void *) Item_getCraftingRemainingItem_injection);

    // Replace 2011 std::sort With Optimized(TM) Code
    if (feature_has("Optimized Chunk Sorting", server_enabled)) {
        overwrite((void *) 0x51fac, (void *) sort_chunks);
    }

    // Display Date In Select World Screen
    if (feature_has("Display Date In Select World Screen", server_disabled)) {
        patch_address(AppPlatform_linux_getDateString_vtable_addr, (void *) AppPlatform_linux_getDateString_injection);
    }
}
