#include <string>
#include <fstream>
#include <streambuf>

#include <cstring>

#include <libreborn/libreborn.h>

#include "../feature/feature.h"
#include "misc.h"

#include <libreborn/minecraft.h>

// Read Asset File
static AppPlatform_readAssetFile_return_value AppPlatform_readAssetFile_injection(__attribute__((unused)) unsigned char *app_platform, std::string const& path) {
    // Read File
    std::string full_path("./data/");
    full_path.append(path);
    std::ifstream stream(full_path);
    std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    // Return String
    AppPlatform_readAssetFile_return_value ret;
    ret.length = str.length();
    ret.data = strdup(str.c_str());
    return ret;
}

// Add Item To Inventory
static void inventory_add_item(unsigned char *inventory, unsigned char *item, bool is_tile) {
    unsigned char *item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
    ALLOC_CHECK(item_instance);
    item_instance = (*(is_tile ? ItemInstance_constructor_tile : ItemInstance_constructor_item))(item_instance, item);
    (*FillingContainer_addItem)(inventory, item_instance);
}

// Expand Creative Inventory
static int32_t Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container, unsigned char *item_instance) {
    // Call Original
    int32_t ret = (*FillingContainer_addItem)(filling_container, item_instance);

    // Add Items
    inventory_add_item(filling_container, *Item_flintAndSteel, false);
    inventory_add_item(filling_container, *Item_snowball, false);
    inventory_add_item(filling_container, *Item_egg, false);
    inventory_add_item(filling_container, *Item_shears, false);
    for (int i = 0; i < 16; i++) {
        if (i == 15) {
            // Bonemeal Is Already In The Creative Inventory
            continue;
        }
        unsigned char *item_instance = (unsigned char *) ::operator new(ITEM_INSTANCE_SIZE);
        ALLOC_CHECK(item_instance);
        item_instance = (*ItemInstance_constructor_item_extra)(item_instance, *Item_dye_powder, 1, i);
        (*FillingContainer_addItem)(filling_container, item_instance);
    }
    inventory_add_item(filling_container, *Item_camera, false);
    // Add Tiles
    inventory_add_item(filling_container, *Tile_water, true);
    inventory_add_item(filling_container, *Tile_lava, true);
    inventory_add_item(filling_container, *Tile_calmWater, true);
    inventory_add_item(filling_container, *Tile_calmLava, true);
    inventory_add_item(filling_container, *Tile_glowingObsidian, true);
    inventory_add_item(filling_container, *Tile_web, true);
    inventory_add_item(filling_container, *Tile_topSnow, true);
    inventory_add_item(filling_container, *Tile_ice, true);
    inventory_add_item(filling_container, *Tile_invisible_bedrock, true);

    return ret;
}

// Print Chat To Log
static bool Gui_addMessage_recursing = false;
static void Gui_addMessage_injection(unsigned char *gui, std::string const& text) {
    // Sanitize Message
    char *new_message = strdup(text.c_str());
    ALLOC_CHECK(new_message);
    sanitize_string(&new_message, -1, 1);

    // Process Message
    if (!Gui_addMessage_recursing) {
        // Start Recursing
        Gui_addMessage_recursing = true;

        // Print Log Message
        fprintf(stderr, "[CHAT]: %s\n", new_message);

        // Call Original Method
        (*Gui_addMessage)(gui, std::string(new_message));

        // End Recursing
        Gui_addMessage_recursing = false;
    } else {
        // Call Original Method
        (*Gui_addMessage)(gui, std::string(new_message));
    }

    // Free
    free(new_message);
}

// Death Messages
static std::string get_death_message(unsigned char *player) {
    // Get Username
    char *username = *(char **) (player + Player_username_property_offset);

    // Prepare Death Message
    std::string message;
    message.append(username);
    message.append(" has died");

    // Return
    return message;
}
// Common Death Message Logic
static void Player_actuallyHurt_injection_helper(unsigned char *player, int32_t damage, bool is_local_player) {
    // Store Old Health
    int32_t old_health = *(int32_t *) (player + Mob_health_property_offset);

    // Call Original Method
    (*(is_local_player ? LocalPlayer_actuallyHurt : Mob_actuallyHurt))(player, damage);

    // Store New Health
    int32_t new_health = *(int32_t *) (player + Mob_health_property_offset);

    // Get Variables
    unsigned char *minecraft = *(unsigned char **) (player + (is_local_player ? LocalPlayer_minecraft_property_offset : ServerPlayer_minecraft_property_offset));
    unsigned char *rak_net_instance = *(unsigned char **) (minecraft + Minecraft_rak_net_instance_property_offset);
    unsigned char *rak_net_instance_vtable = *(unsigned char **) rak_net_instance;
    // Only Run On Server-Side
    RakNetInstance_isServer_t RakNetInstance_isServer = *(RakNetInstance_isServer_t *) (rak_net_instance_vtable + RakNetInstance_isServer_vtable_offset);
    if ((*RakNetInstance_isServer)(rak_net_instance)) {
        // Check Health
        if (new_health < 1 && old_health >= 1) {
            // Get Death Message
            std::string message = get_death_message(player);

            // Post Death Message
            unsigned char *server_side_network_handler = *(unsigned char **) (minecraft + Minecraft_network_handler_property_offset);
            (*ServerSideNetworkHandler_displayGameMessage)(server_side_network_handler, message);
        }
    }
}
// ServerPlayer Death Message Logic
static void ServerPlayer_actuallyHurt_injection(unsigned char *player, int32_t damage) {
    Player_actuallyHurt_injection_helper(player, damage, false);
}
// LocalPlayer Death Message Logic
static void LocalPlayer_actuallyHurt_injection(unsigned char *player, int32_t damage) {
    Player_actuallyHurt_injection_helper(player, damage, true);
}

void init_misc_cpp() {
    // Implement AppPlatform::readAssetFile So Translations Work
    overwrite((void *) AppPlatform_readAssetFile, (void *) AppPlatform_readAssetFile_injection);

    if (feature_has("Expand Creative Inventory")) {
        // Add Extra Items To Creative Inventory (Only Replace Specific Function Call)
        overwrite_call((void *) 0x8e0fc, (void *) Inventory_setupDefault_FillingContainer_addItem_call_injection);
    }

    // Print Chat To Log
    overwrite_calls((void *) Gui_addMessage, (void *) Gui_addMessage_injection);

    // Death Messages
    patch_address(ServerPlayer_actuallyHurt_vtable_addr, (void *) ServerPlayer_actuallyHurt_injection);
    patch_address(LocalPlayer_actuallyHurt_vtable_addr, (void *) LocalPlayer_actuallyHurt_injection);
}