#include <string>

#include <libreborn/patch.h>

#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/misc/misc.h>

// Death Messages
std::string get_death_message(Player *player, Entity *cause, const bool was_shot = false) {
    // Prepare Death Message
    std::string message = player->username;
    if (cause) {
        // Entity Cause
        const int type_id = cause->getEntityTypeId();
        const int aux = cause->getAuxData();
        const bool is_player = cause->isPlayer();
        if (cause->getCreatureBaseType() != 0 || is_player) {
            // Killed By A Creature
            if (was_shot) {
                message += " was shot by ";
            } else {
                message += " was killed by ";
            }
            if (is_player) {
                // Killed By A Player
                message += ((Player *) cause)->username;
            } else {
                // Normal Creature
                std::string type = misc_get_entity_type_name(cause).first;
                if (type.empty()) {
                    type = "mysterious beast";
                }
                message += "a " + type;
            }
            return message;
        } else if (aux) {
            // Killed By A Throwable With Owner
            Level *level = player->level;
            Entity *shooter = level->getEntity(aux);
            return get_death_message(player, shooter, true);
        } else if (type_id == 65) {
            // Blown up by TNT
            return message + " was blown apart";
        } else if (cause->isHangingEntity()) {
            // Painting?
            return message + " admired too much art";
        }
    }
    // Miscellaneous Causes
    if (was_shot) {
        // Throwable With Invalid Owner
        return message + " was shot under mysterious circumstances";
    } else if (cause) {
        // Unknown Entity
        return message + " was killed";
    } else {
        // Anything Else
        return message + " has died";
    }
}

// Track If A Mob Is Being Hurt
static bool is_hurt = false;
static bool Mob_hurt_injection(Mob_hurt_t original, Mob *mob, Entity *source, const int dmg) {
    is_hurt = true;
    const bool ret = original(mob, source, dmg);
    is_hurt = false;
    return ret;
}

// Death Message Logic
template <typename Self, typename ParentSelf>
static void Player_die_injection(const std::function<void(ParentSelf *, Entity *)> &original, Self *player, Entity *cause) {
    // Call Original Method
    original((ParentSelf *) player, cause);

    // Get Variable
    RakNetInstance *rak_net_instance = player->level->rak_net_instance;
    // Only Run On The Server-Side
    if (rak_net_instance->isServer()) {
        // Get Death Message
        const std::string message = get_death_message((Player *) player, cause);

        // Post The Death Message
        ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) player->minecraft->network_handler;
        server_side_network_handler->displayGameMessage(message);
    }
}
template <typename OriginalSelf, typename Self>
static void Player_actuallyHurt_injection(const std::function<void(OriginalSelf *, int)> &original, Self *player, int32_t damage) {
    // Store Old Health
    const int32_t old_health = player->health;

    // Call Original Method
    original((OriginalSelf *) player, damage);
    if (is_hurt) {
        return;
    }

    // Store New Health
    const int32_t new_health = player->health;

    // Get Variables
    RakNetInstance *rak_net_instance = player->level->rak_net_instance;
    // Only Run On The Server-Side
    if (rak_net_instance->isServer()) {
        // Check Health
        if (new_health < 1 && old_health >= 1) {
            // Get Death Message
            const std::string message = get_death_message((Player *) player, nullptr);

            // Post The Death Message
            ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) player->minecraft->network_handler;
            server_side_network_handler->displayGameMessage(message);
        }
    }
}
static void ServerPlayer_actuallyHurt_injection(ServerPlayer *player, const int32_t damage) {
    Player_actuallyHurt_injection<Mob, ServerPlayer>(Mob_actuallyHurt->get(false), player, damage);
}
static void LocalPlayer_actuallyHurt_injection(LocalPlayer_actuallyHurt_t original, LocalPlayer *player, const int32_t damage) {
    Player_actuallyHurt_injection(original, player, damage);
}

// Init
void init_death() {
    // Death Messages
    if (feature_has("Implement Death Messages", server_auto)) {
        patch_vtable(ServerPlayer_die, [](ServerPlayer *player, Entity *cause) {
            Player_die_injection<ServerPlayer, Player>(Player_die->get(false), player, cause);
        });
        overwrite_calls(LocalPlayer_die, Player_die_injection<LocalPlayer, LocalPlayer>);
        overwrite_calls(LocalPlayer_actuallyHurt, LocalPlayer_actuallyHurt_injection);
        patch_vtable(ServerPlayer_actuallyHurt, ServerPlayer_actuallyHurt_injection);
        overwrite_calls(Mob_hurt, Mob_hurt_injection);

        // Fix TNT
        // This changes PrimedTnt_explode from Level::explode(nullptr, x, y, z, 3.1f) to Level::explode(this, x, y, z, 3.1f)
        unsigned char cpy_r1_r0_patch[4] = {0x00, 0x10, 0xa0, 0xe1}; // "cpy r1, r0"
        patch((void *) 0x87998, cpy_r1_r0_patch);
        unsigned char ldr_r0_24_patch[4] = {0x24, 0x00, 0x90, 0xe5}; // "ldr r0, [r0, #0x24]"
        patch((void *) 0x8799c, ldr_r0_24_patch);
    }
}
