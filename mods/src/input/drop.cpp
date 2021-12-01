#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "input.h"
#include "../feature/feature.h"
#include "../creative/creative.h"

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
static void _handle_drop(unsigned char *minecraft) {
    if ((!creative_is_restricted() || !(*Minecraft_isCreativeMode)(minecraft)) && (drop_item_presses > 0 || drop_slot_pressed)) {
        // Get Player
        unsigned char *player = *(unsigned char **) (minecraft + Minecraft_player_property_offset);
        if (player != NULL) {
            // Get Selected Slot
            unsigned char *inventory = *(unsigned char **) (player + Player_inventory_property_offset);
            int32_t selected_slot = *(int32_t *) (inventory + Inventory_selectedSlot_property_offset);

            // Prepare
            unsigned char *player_vtable = *(unsigned char **) player;
            Player_drop_t Player_drop = *(Player_drop_t *) (player_vtable + Player_drop_vtable_offset);
            unsigned char *inventory_vtable = *(unsigned char **) inventory;
            FillingContainer_getItem_t FillingContainer_getItem = *(FillingContainer_getItem_t *) (inventory_vtable + FillingContainer_getItem_vtable_offset);

            // Linked Slots
            int32_t linked_slots_length = *(int32_t *) (inventory + FillingContainer_linked_slots_length_property_offset);
            if (selected_slot < linked_slots_length) {
                int32_t *linked_slots = *(int32_t **) (inventory + FillingContainer_linked_slots_property_offset);
                selected_slot = linked_slots[selected_slot];
            }

            // Get Item
            ItemInstance *inventory_item = (*FillingContainer_getItem)(inventory, selected_slot);
            // Check
            if (inventory_item != NULL && inventory_item->count > 0) {
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
                    (*FillingContainer_release)(inventory, selected_slot);
                    (*FillingContainer_compressLinkedSlotList)(inventory, selected_slot);
                }

                // Drop
                (*Player_drop)(player, dropped_item, false);
            }
        }
    }
    // Reset
    drop_item_presses = 0;
    drop_slot_pressed = false;
}

// Init
void _init_drop() {
    enable_drop = feature_has("Bind \"Q\" Key To Item Dropping", 0);
    input_run_on_tick(_handle_drop);
}
