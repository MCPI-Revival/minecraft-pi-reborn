#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "input-internal.h"
#include <mods/input/input.h>
#include <mods/feature/feature.h>
#include <mods/creative/creative.h>
#include <mods/misc/misc.h>

// Track Control Key
static bool drop_slot = false;
void input_set_is_ctrl(const bool val) {
    drop_slot = val;
}

// Handle Drop Item Presses
static void _handle_drop(Minecraft *minecraft) {
    if (!creative_is_restricted() || !Minecraft_isCreativeMode(minecraft)) {
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
                if (drop_slot) {
                    // Drop Entire Slot
                    inventory_item->count = 0;
                } else {
                    // Drop Item
                    const int drop_count = 1;
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
}

// Init
void _init_drop() {
    if (feature_has("Bind \"Q\" Key To Item Dropping", server_disabled)) {
        misc_run_on_game_key_press([](Minecraft *mc, int key) {
            if (key == MC_KEY_q) {
                _handle_drop(mc);
                return true;
            } else {
                return false;
            }
        });
    }
}
