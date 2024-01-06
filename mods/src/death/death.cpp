#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>

// Death Messages
static std::string get_death_message(Player *player) {
    // Get Username
    std::string *username = &player->username;

    // Prepare Death Message
    std::string message;
    message.append(username->c_str());
    message.append(" has died");

    // Return
    return message;
}

// Death Message Logic
#define Player_actuallyHurt_injection(type) \
    static void type##Player_actuallyHurt_injection(type##Player *player, int32_t damage) { \
        /* Store Old Health */ \
        int32_t old_health = player->health; \
        \
        /* Call Original Method */ \
        (*type##Player_actuallyHurt_non_virtual)(player, damage); \
        \
        /* Store New Health */ \
        int32_t new_health = player->health; \
        \
        /* Get Variables */ \
        Minecraft *minecraft = player->minecraft; \
        RakNetInstance *rak_net_instance = minecraft->rak_net_instance; \
        /* Only Run On Server-Side */ \
        if (rak_net_instance->vtable->isServer(rak_net_instance)) { \
            /* Check Health */ \
            if (new_health < 1 && old_health >= 1) { \
                /* Get Death Message */ \
                std::string message = get_death_message((Player *) player); \
                \
                /* Post Death Message */ \
                ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) minecraft->network_handler; \
                (*ServerSideNetworkHandler_displayGameMessage)(server_side_network_handler, &message); \
            } \
        } \
    }
Player_actuallyHurt_injection(Local)
Player_actuallyHurt_injection(Server)

// Init
void init_death() {
    // Death Messages
    if (feature_has("Implement Death Messages", server_auto)) {
        patch_address(ServerPlayer_actuallyHurt_vtable_addr, (void *) ServerPlayer_actuallyHurt_injection);
        patch_address(LocalPlayer_actuallyHurt_vtable_addr, (void *) LocalPlayer_actuallyHurt_injection);
    }
}
