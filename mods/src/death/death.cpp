#include <string>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>

// Death Messages
static const char *monster_names[] = {"Zombie", "Creeper", "Skeleton", "Spider", "Zombie Pigman"};
std::string get_death_message(Player *player, Entity *cause, bool was_shot = false) {
    // Prepare Death Message
    std::string message = player->username;
    if (cause) {
        // Entity cause
        int type_id = cause->vtable->getEntityTypeId(cause);
        int aux = cause->vtable->getAuxData(cause);
        bool is_player = cause->vtable->isPlayer(cause);
        if (cause->vtable->getCreatureBaseType(cause) != 0 || is_player) {
            // Killed by a creature
            if (was_shot) {
                message += " was shot by ";
            } else {
                message += " was killed by ";
            }
            if (is_player) {
                // Killed by a player
                message += ((Player *) cause)->username;
            } else if (32 <= type_id && type_id <= 36) {
                // Normal monster
                message += "a ";
                message += monster_names[type_id - 32];
            } else {
                // Unknown creature
                message += "a Mysterious Beast";
            }
            return message;
        } else if (aux) {
            // Killed by a throwable with owner
            Level *level = player->level;
            Entity *shooter = Level_getEntity(level, aux);
            return get_death_message(player, shooter, true);
        } else if (type_id == 65) {
            // Blown up by TNT
            return message + " was blown apart";
        } else if (cause->vtable->isHangingEntity(cause)) {
            // Painting?
            return message + " admired too much art";
        }
    }

    if (was_shot) {
        // Throwable with invalid owner
        return message + " was shot under mysterious circumstances";
    } else if (cause) {
        // Unknown entity
        return message + " was killed";
    } else {
        // Anything else
        return message + " has died";
    }

    // Return
    return message;
}

static bool is_hurt = false;
static bool Mob_hurt_injection(Mob_hurt_t original, Mob *mob, Entity *source, int dmg) {
    is_hurt = true;
    bool ret = original(mob, source, dmg);
    is_hurt = false;
    return ret;
}

// Death Message Logic
#define Player_die_injections(type) \
    static type##_die_t original_##type##_die; \
    static void type##_die_injection(type *player, Entity *cause) { \
        /* Call Original Method */ \
        original_##type##_die(player, cause); \
        \
        /* Get Variable */ \
        RakNetInstance *rak_net_instance = player->minecraft->rak_net_instance; \
        /* Only Run On Server-Side */ \
        if (rak_net_instance->vtable->isServer(rak_net_instance)) { \
            /* Get Death Message */ \
            std::string message = get_death_message((Player *) player, cause); \
            \
            /* Post Death Message */ \
            ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) player->minecraft->network_handler; \
            ServerSideNetworkHandler_displayGameMessage(server_side_network_handler, &message); \
        } \
    }
#define Player_actuallyHurt_injections(type) \
    static void type##_actuallyHurt_injection(type *player, int32_t damage) { \
        /* Store Old Health */ \
        int32_t old_health = player->health; \
        \
        /* Call Original Method */ \
        (*Mob_actuallyHurt_vtable_addr)((Mob *) player, damage); \
        if (is_hurt == true) return; \
        \
        /* Store New Health */ \
        int32_t new_health = player->health; \
        \
        /* Get Variables */ \
        RakNetInstance *rak_net_instance = player->minecraft->rak_net_instance; \
        /* Only Run On Server-Side */ \
        if (rak_net_instance->vtable->isServer(rak_net_instance)) { \
            /* Check Health */ \
            if (new_health < 1 && old_health >= 1) { \
                /* Get Death Message */ \
                std::string message = get_death_message((Player *) player, nullptr); \
                \
                /* Post Death Message */ \
                ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) player->minecraft->network_handler; \
                ServerSideNetworkHandler_displayGameMessage(server_side_network_handler, &message); \
            } \
        } \
    }

Player_die_injections(LocalPlayer)
Player_die_injections(ServerPlayer)

Player_actuallyHurt_injections(LocalPlayer)
Player_actuallyHurt_injections(ServerPlayer)

// Init
void init_death() {
    // Death Messages
    if (feature_has("Implement Death Messages", server_auto)) {
        patch_address(ServerPlayer_die_vtable_addr, ServerPlayer_die_injection);
        patch_address(LocalPlayer_die_vtable_addr, LocalPlayer_die_injection);
        patch_address(LocalPlayer_actuallyHurt_vtable_addr, LocalPlayer_actuallyHurt_injection);
        patch_address(ServerPlayer_actuallyHurt_vtable_addr, ServerPlayer_actuallyHurt_injection);
        overwrite_virtual_calls(Mob_hurt, Mob_hurt_injection);
    }

    // Fix TNT
    // This changes PrimedTnt_explode from Level::explode(nullptr, x, y, z, 3.1f) to Level::explode(this, x, y, z, 3.1f)
    unsigned char cpy_r1_r0_patch[4] = {0x00, 0x10, 0xa0, 0xe1}; // "cpy r1, r0"
    patch((void *) 0x87998, cpy_r1_r0_patch);
    unsigned char ldr_r0_24_patch[4] = {0x24, 0x00, 0x90, 0xe5}; // "ldr r0, [r0, #0x24]"
    patch((void *) 0x8799c, ldr_r0_24_patch);
}
