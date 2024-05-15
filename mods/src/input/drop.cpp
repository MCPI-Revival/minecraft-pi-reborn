#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "input-internal.h"
#include <mods/input/input.h>
#include <mods/feature/feature.h>
#include <mods/creative/creative.h>
#include <mods/misc/misc.h>

// Enable Item Dropping
static int enable_drop = 0;

// Store Drop Item Presses
static int drop_item_presses = 0;
static bool drop_slot_pressed = false;
void input_drop(int drop_slot) {
    if (enable_drop) {
        if (drop_slot) {
            drop_slot_pressed = true;
        } else {
            drop_item_presses++;
        }
    }
}

// Handle Drop Item Presses
static void _handle_drop(Minecraft *minecraft) {
    if ((minecraft->screen == nullptr) && (!creative_is_restricted() || !Minecraft_isCreativeMode(minecraft)) && (drop_item_presses > 0 || drop_slot_pressed)) {
        // Get Player
        LocalPlayer *player = minecraft->player;
        if (player != nullptr) {
            // Get Selected Slot
            int32_t selected_slot = misc_get_real_selected_slot((Player *) player);
            Inventory *inventory = player->inventory;

            // Get Item
            ItemInstance *inventory_item = inventory->getItem(selected_slot);
            // Check
            if (inventory_item != nullptr && inventory_item->count > 0) {
                // Copy
                ItemInstance *dropped_item = new ItemInstance;
                ALLOC_CHECK(dropped_item);
                *dropped_item = *inventory_item;

                // Update Inventory
                if (drop_slot_pressed) {
                    // Drop Slot

                    // Empty Slot
                    inventory_item->count = 0;
                } else {
                    // Drop Item

                    // Set Item Drop Count
                    int drop_count = drop_item_presses < inventory_item->count ? drop_item_presses : inventory_item->count;
                    dropped_item->count = drop_count;
                    inventory_item->count -= drop_count;
                }

                // Empty Slot If Needed
                if (inventory_item->count < 1) {
                    inventory->release(selected_slot);
                    inventory->compressLinkedSlotList(selected_slot);
                }

                // Drop
                player->drop(dropped_item, false);
            }
        }
    }
    // Reset
    drop_item_presses = 0;
    drop_slot_pressed = false;
}

// Init
void _init_drop() {
    enable_drop = feature_has("Bind \"Q\" Key To Item Dropping", server_disabled);
    input_run_on_tick(_handle_drop);
}
