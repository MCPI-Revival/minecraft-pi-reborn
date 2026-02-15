#include "internal.h"

#include <cmath>

#include <libreborn/util/string.h>

#include <symbols/CommandServer.h>
#include <symbols/Minecraft.h>
#include <symbols/Level.h>
#include <symbols/Entity.h>
#include <symbols/Mob.h>
#include <symbols/ItemInstance.h>
#include <symbols/ItemEntity.h>
#include <symbols/SetEntityMotionPacket.h>
#include <symbols/Packet.h>
#include <symbols/RakNetInstance.h>

#include <mods/misc/misc.h>
#include <mods/api/api.h>

// Set Entity Rotation From XYZ
static void set_dir(Entity *entity, const float x, const float y, const float z) {
    // Check Rotation
    if (entity == nullptr) {
        return;
    }
    // Calculate
    if (x == 0 && z == 0) {
        entity->pitch = y > 0 ? -90 : 90;
        return;
    }
    constexpr float _2PI = 2 * M_PI;
    constexpr float factor = float(180.0f / M_PI);
    entity->yaw = std::fmod(std::atan2(-x, z), _2PI) * factor;
    const float xz = std::sqrt(x * x + z * z);
    entity->pitch = std::atan(-y / xz) * factor;
    api_update_entity_position(entity);
}
// Convert Entity Rotation To XYZ
static Vec3 get_dir(const Entity *entity) {
    constexpr float factor = float(M_PI / 180);
    const float y = -std::sin(entity->pitch * factor);
    const float xz = std::cos(entity->pitch * factor);
    const float x = -xz * std::sin(entity->yaw * factor);
    const float z = xz * std::cos(entity->yaw * factor);
    return Vec3{x, y, z};
}

// Convert Entity To String
std::string api_get_entity_message(CommandServer *server, Entity *entity) {
    std::vector<std::string> pieces;
    // ID
    pieces.push_back(safe_to_string(entity->id));
    // Type
    int type = entity->getEntityTypeId();
    api_convert_to_outside_entity_type(type);
    pieces.push_back(safe_to_string(type));
    if (api_compat_mode) {
        pieces.push_back(api_get_output(misc_get_entity_type_name(entity).second, true));
    }
    // XYZ
    float x = entity->x;
    float y = entity->y - entity->height_offset;
    float z = entity->z;
    server->pos_translator.to_float(x, y, z);
    pieces.push_back(safe_to_string(x));
    pieces.push_back(safe_to_string(y));
    pieces.push_back(safe_to_string(z));
    // Return
    return api_join_outputs(pieces, arg_separator);
}

// Calculate Distance Between Entities
static float distance_between(const Entity *e1, const Entity *e2) {
    if (e1 == nullptr || e2 == nullptr) {
        return -1;
    }
    const float dx = e2->x - e1->x;
    const float dy = e2->y - e1->y;
    const float dz = e2->z - e1->z;
    return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

// Check If The Entity Is Selected
bool api_is_entity_selected(const Entity *entity, const int target_type) {
    const int type = entity->getEntityTypeId();
    return type > 0 && (target_type == no_entity_id || target_type == type);
}

// Get Selected Item
static const ItemInstance *get_selected_item(Entity *entity) {
    const ItemInstance *item = nullptr;
    if (entity->isMob()) {
        // Mob/Player
        item = ((Mob *) entity)->getCarriedItem();
        if (!item) {
            static ItemInstance air = {0, 0, 0};
            item = &air;
        }
    } else if (entity->getEntityTypeId() == static_cast<int>(EntityType::DROPPED_ITEM)) {
        // Dropped Item
        item = &((ItemEntity *) entity)->item;
    }
    return item;
}

// Handle Entity Commands
std::string api_handle_entity_command(const std::function<std::string()> &super, CommandServer *server, const ConnectedClient &client, std::string_view &cmd, std::istringstream &args) {
    // Vanilla Commands
    passthrough(setTile);
    passthrough(getTile);
    passthrough(setPos);
    passthrough(getPos);

    // Common Code For New Commands
    next_int(id);
    Entity *entity = server->minecraft->level->getEntity(id);

    // Name/Type/ID
    command(getName) {
        if (!entity) {
            // Matching RJ Behavior, Even Though It Is Dumb
            return CommandServer::NullString;
        }
        return api_get_output(misc_get_entity_name(entity), false) + '\n';
    }
    command(getType) {
        // Reborn Extension
        if (!entity) {
            return CommandServer::Fail;
        }
        int type = entity->getEntityTypeId();
        api_convert_to_outside_entity_type(type);
        return safe_to_string(type) + '\n';
    }
    command(getId) {
        // Reborn Extension
        if (!entity) {
            return CommandServer::Fail;
        }
        return safe_to_string(entity->id) + '\n'; // I Promise This Is Useful
    }

    // Get/Remove Nearby Entities
    command(getEntities) {
        // Parse
        // This skips the entity null-check to match RJ.
        next_int(dist);
        next_int(type);
        api_convert_to_mcpi_entity_type(type);
        // Run
        std::vector<std::string> result;
        for (Entity *other : server->minecraft->level->entities) {
            if (api_is_entity_selected(other, type) && distance_between(entity, other) <= dist) {
                result.push_back(api_get_entity_message(server, other));
            }
        }
        return api_join_outputs(result, list_separator);
    }
    command(removeEntities) {
        // Parse
        // This skips the entity null-check to match RJ.
        next_int(dist);
        next_int(type);
        api_convert_to_mcpi_entity_type(type);
        // Run
        int removed = 0;
        for (Entity *other : server->minecraft->level->entities) {
            if (api_is_entity_selected(other, type) && distance_between(entity, other) <= dist) {
                other->remove();
                removed++;
            }
        }
        return safe_to_string(removed) + '\n';
    }

    // Get/Set Rotation
    command(setDirection) {
        // Parse
        if (!entity) {
            return CommandServer::NullString;
        }
        next_float(x);
        next_float(y);
        next_float(z);
        // Set
        set_dir(entity, x, y, z);
        return CommandServer::NullString;
    }
    command(getDirection) {
        // Parse
        if (!entity) {
            return CommandServer::Fail;
        }
        // Get
        Vec3 vec = get_dir(entity);
        return api_join_outputs({safe_to_string(vec.x), safe_to_string(vec.y), safe_to_string(vec.z)}, arg_separator);
    }
    command(setRotation) {
        // Parse
        if (!entity) {
            return CommandServer::NullString;
        }
        next_float(yaw);
        // Set
        entity->yaw = yaw;
        api_update_entity_position(entity);
        return CommandServer::NullString;
    }
    command(getRotation) {
        // Parse
        if (!entity) {
            return CommandServer::Fail;
        }
        // Get
        return safe_to_string(entity->yaw) + '\n';
    }
    command(setPitch) {
        // Parse
        if (!entity) {
            return CommandServer::NullString;
        }
        next_float(pitch);
        // Set
        entity->pitch = pitch;
        api_update_entity_position(entity);
        return CommandServer::NullString;
    }
    command(getPitch) {
        // Parse
        if (!entity) {
            return CommandServer::Fail;
        }
        // Get
        return safe_to_string(entity->pitch) + '\n';
    }

    // Get/Set Absolute Position
    command(setAbsPos) {
        // Parse
        if (!entity) {
            return CommandServer::NullString;
        }
        next_float(x);
        next_float(y);
        next_float(z);
        // Set
        entity->moveTo(x, y, z, entity->yaw, entity->pitch);
        api_update_entity_position(entity);
        return CommandServer::NullString;
    }
    command(getAbsPos) {
        // Parse
        if (!entity) {
            return CommandServer::Fail;
        }
        // Get
        return api_join_outputs({safe_to_string(entity->x), safe_to_string(entity->y - entity->height_offset), safe_to_string(entity->z)}, arg_separator);
    }

    // Get/Set Velocity
    command(setVelocity) {
        // Get Entity
        if (!entity) {
            return CommandServer::NullString;
        }
        // Set Velocity
        SetEntityMotionPacket *packet = SetEntityMotionPacket::allocate();
        ((Packet *) packet)->constructor();
        packet->vtable = SetEntityMotionPacket::VTable::base;
        packet->entity_id = id;
#define next_component(axis) \
    next_float(axis); \
    entity->velocity_##axis = packet->velocity_##axis = axis
        next_component(x);
        next_component(y);
        next_component(z);
#undef next_component
        // Send Packet
        entity->level->rak_net_instance->send(*(Packet *) packet);
        packet->destructor_deleting();
        return CommandServer::NullString;
    }
    command(getVelocity) {
        // Parse
        if (!entity) {
            return CommandServer::Fail;
        }
        // Get
        return api_join_outputs({safe_to_string(entity->velocity_x), safe_to_string(entity->velocity_y), safe_to_string(entity->velocity_z)}, arg_separator);
    }

    // Selected Item
    command(getSelectedItem) {
        // Parse
        if (!entity) {
            return CommandServer::Fail;
        }
        // Get Item
        const ItemInstance *item = get_selected_item(entity);
        if (!item) {
            // Entity Does Not Carry Items
            return CommandServer::Fail;
        }
        // Return
        return api_join_outputs({
            safe_to_string(item->id),
            safe_to_string(item->count),
            safe_to_string(item->auxiliary)
        }, arg_separator);
    }
#undef get_entity
#undef _get_entity

    // Handle Events
    package(events) {
        return api_handle_event_command(server, client, cmd, id);
    }

    // Invalid Command
    return CommandServer::Fail;
}