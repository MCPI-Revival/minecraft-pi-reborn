#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include "../init/init.h"
#include "../feature/feature.h"

// Death Messages
static std::string get_death_message(unsigned char *player) {
    // Get Username
    std::string *username = (std::string *) (player + Player_username_property_offset);

    // Prepare Death Message
    std::string message;
    message.append(username->c_str());
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

// Init
void init_death() {
    // Death Messages
    if (feature_has("Implement Death Messages", 1)) {
        patch_address(ServerPlayer_actuallyHurt_vtable_addr, (void *) ServerPlayer_actuallyHurt_injection);
        patch_address(LocalPlayer_actuallyHurt_vtable_addr, (void *) LocalPlayer_actuallyHurt_injection);
    }
}
