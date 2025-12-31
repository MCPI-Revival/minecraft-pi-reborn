#include <libreborn/patch.h>

#include <symbols/Minecraft.h>
#include <symbols/LocalPlayer.h>
#include <symbols/Inventory.h>

#include "internal.h"
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
    // Use The Same Check As Gui::tick
    typedef std::remove_pointer_t<decltype(Minecraft_isCreativeMode)>::ptr_type func_t;
    static func_t is_creative_mode = (func_t) extract_from_bl_instruction((uchar *) Gui_tick_Minecraft_isCreativeMode_addr);
    if (is_creative_mode(minecraft)) {
        return;
    }

    // Get Player
    LocalPlayer *player = minecraft->player;
    if (!player) {
        return;
    }

    // Get Selected Slot
    const int32_t selected_slot = misc_get_real_selected_slot((Player *) player);
    Inventory *inventory = player->inventory;
    // Get Item
    ItemInstance *inventory_item = inventory->getItem(selected_slot);
    if (!inventory_item || inventory_item->count <= 0) {
        return;
    }

    // Copy Item
    ItemInstance *dropped_item = new ItemInstance;
    *dropped_item = *inventory_item;

    // Update Inventory
    if (drop_slot) {
        // Drop The Entire Slot
        inventory_item->count = 0;
    } else {
        // Drop Item
        constexpr int drop_count = 1;
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

// Init
void _init_drop() {
    if (feature_has("Bind 'Q' Key To Item Dropping", server_disabled)) {
        misc_run_on_game_key_press([](Minecraft *mc, const int key) {
            if (key == MC_KEY_q) {
                _handle_drop(mc);
                return true;
            } else {
                return false;
            }
        });
    }
}
