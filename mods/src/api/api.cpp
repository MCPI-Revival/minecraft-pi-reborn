#include <cmath>
#include <string>
#include <algorithm>
#include <optional>
#include <sstream>

#include <libreborn/log.h>
#include <libreborn/util/string.h>
#include <libreborn/patch.h>
#include <libreborn/config.h>
#include <symbols/minecraft.h>

#include <mods/api/api.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Compatibility Mode
bool api_compat_mode = true;

// Read String Input
static std::string get_input(std::string message) {
    // Decode
    if (!api_compat_mode) {
        message = misc_base64_decode(message);
    }
    // Convert To CP-437
    return to_cp437(message);
}
// Output String
std::string api_get_output(std::string message, const bool replace_comma) {
    // Convert To Unicode
    message = from_cp437(message);
    // Escape Characters
    if (api_compat_mode) {
        // Output In Plaintext For RJ Compatibility
        std::ranges::replace(message, list_separator, '\\');
        if (replace_comma) {
            std::ranges::replace(message, arg_separator, '.');
        }
    } else {
        // Encode
        message = misc_base64_encode(message);
    }
    // Return
    return message;
}

// Join Strings Into Output
std::string api_join_outputs(const std::vector<std::string> &pieces, const char separator) {
    // Join
    std::string out;
    for (std::string piece : pieces) {
        // Check
        if (piece.find(separator) != std::string::npos) {
            // This Should Be Escapes
            IMPOSSIBLE();
        }
        // Remove Trailing Newline
        if (!piece.empty() && piece.back() == '\n') {
            piece.pop_back();
        }
        // Add
        out += piece + separator;
    }
    // Remove Hanging Comma
    if (!out.empty()) {
        out.pop_back();
    }
    // Return
    return out + '\n';
}

// Get Blocks In Region
static std::string get_blocks(CommandServer *server, const Vec3 &start, const Vec3 &end) {
    // Start Coordinate
    int start_x = int(start.x);
    int start_y = int(start.y);
    int start_z = int(start.z);
    // End Coordinate
    int end_x = int(end.x);
    int end_y = int(end.y);
    int end_z = int(end.z);

    // Apply Offset
    server->pos_translator.from(start_x, start_y, start_z);
    server->pos_translator.from(end_x, end_y, end_z);

    // Swap If Needed
    if (end_x < start_x) {
        std::swap(start_x, end_x);
    }
    if (end_y < start_y) {
        std::swap(start_y, end_y);
    }
    if (end_z < start_z) {
        std::swap(start_z, end_z);
    }

    // Get
    std::vector<std::string> ret;
    for (int x = start_x; x <= end_x; x++) {
        for (int y = start_y; y <= end_y; y++) {
            for (int z = start_z; z <= end_z; z++) {
                ret.push_back(std::to_string(server->minecraft->level->getTile(x, y, z)));
            }
        }
    }
    // Return
    return api_join_outputs(ret, arg_separator);
}

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

// Entity Types
static std::unordered_map<int, EntityType> modern_entity_id_mapping = {
    {93, EntityType::CHICKEN},
    {92, EntityType::COW},
    {90, EntityType::PIG},
    {91, EntityType::SHEEP},
    {54, EntityType::ZOMBIE},
    {50, EntityType::CREEPER},
    {51, EntityType::SKELETON},
    {52, EntityType::SPIDER},
    {57, EntityType::ZOMBIE_PIGMAN},
    {1, EntityType::DROPPED_ITEM},
    {20, EntityType::PRIMED_TNT},
    {21, EntityType::FALLING_SAND},
    {10, EntityType::ARROW},
    {11, EntityType::THROWN_SNOWBALL},
    {7, EntityType::THROWN_EGG},
    {9, EntityType::PAINTING}
};
void api_convert_to_rj_entity_type(int &type) {
    for (const std::pair<const int, EntityType> &pair : modern_entity_id_mapping) {
        if (static_cast<int>(pair.second) == type) {
            type = pair.first;
        }
    }
}
void api_convert_to_mcpi_entity_type(int &type) {
    if (modern_entity_id_mapping.contains(type)) {
        type = static_cast<int>(modern_entity_id_mapping[type]);
    }
}

// Convert Entity To String
static std::string get_entity_message(CommandServer *server, Entity *entity) {
    // Offset Position
    float x = entity->x;
    float y = entity->y - entity->height_offset;
    float z = entity->z;
    server->pos_translator.to(x, y, z);
    // Fix Type ID
    int type = entity->getEntityTypeId();
    if (api_compat_mode) {
        api_convert_to_rj_entity_type(type);
    }
    // Return
    return api_join_outputs({
        // ID
        std::to_string(entity->id),
        // Type
        std::to_string(type),
        // Name
        api_get_output(misc_get_entity_type_name(entity).second, true),
        // X
        std::to_string(x),
        // Y
        std::to_string(y),
        // X
        std::to_string(z)
    }, arg_separator);
}

// Calculate Distance Between Entities
static float distance_between(const Entity *e1, const Entity *e2) {
    if (e1 == nullptr || e2 == nullptr) {
        return 0;
    }
    const float dx = e2->x - e1->x;
    const float dy = e2->y - e1->y;
    const float dz = e2->z - e1->z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Get Sign Tile Entity
static SignTileEntity *get_sign(const CommandServer *server, const int x, const int y, const int z) {
    TileEntity *sign = server->minecraft->level->getTileEntity(x, y, z);
    if (sign != nullptr && sign->type == 4) {
        return (SignTileEntity *) sign;
    } else {
        return nullptr;
    }
}

// Parse API Commands
static const std::string player_namespace = "player.";
#define API_WARN(format, ...) WARN("API: %s: " format, cmd.c_str(), ##__VA_ARGS__)
#define ENTITY_NOT_FOUND API_WARN("Entity Not Found: %i", id)
#define next_string(out, required) \
    std::string out; \
    if (!std::getline(args, out, arg_separator) && (required)) { \
        return CommandServer::Fail; \
    } \
    (void) 0
#define next_number(out, type, func) \
    next_string(out##_str, true); \
    type out; \
    try { \
        out = func(out##_str); \
    } catch (...) { \
        return CommandServer::Fail; \
    } \
    (void) 0
#define next_int(out) next_number(out, int, std::stoi)
#define next_float(out) next_number(out, float, std::stof)
std::string CommandServer_parse_injection(CommandServer_parse_t old, CommandServer *server, ConnectedClient &client, const std::string &command) {
    size_t arg_start = command.find('(');
    if (arg_start == std::string::npos) {
        return CommandServer::Fail;
    }
    std::string cmd = command.substr(0, arg_start);
    size_t cmd_end = command.rfind(')');
    if (cmd_end == std::string::npos) {
        return CommandServer::Fail;
    }
    std::string args_str = command.substr(arg_start + 1, cmd_end - arg_start - 1);

    // Redirect Player Namespace To The Entity One
    if (server->minecraft->player != nullptr && cmd.starts_with(player_namespace) && cmd != "player.setting") {
        cmd = "entity." + cmd.substr(player_namespace.size());
        args_str = std::to_string(server->minecraft->player->id) + arg_separator + args_str;
    }

    // And Now The Big If-Else Chain
    std::stringstream args(args_str);
    if (cmd == "world.getBlocks") {
        next_int(x0);
        next_int(y0);
        next_int(z0);
        next_int(x1);
        next_int(y1);
        next_int(z1);
        // Get The Blocks
        return get_blocks(server, Vec3{(float) x0, (float) y0, (float) z0}, Vec3{(float) x1, (float) y1, (float) z1});
    } else if (cmd == "world.getPlayerId") {
        next_string(input, true);
        std::string username = get_input(input);
        for (Player *player : server->minecraft->level->players) {
            if (misc_get_player_username_utf(player) == username) {
                return std::to_string(player->id) + "\n";
            }
        }
        API_WARN("Player Not Found: %s", username.c_str());
        return CommandServer::Fail;
    } else if (cmd == "entity.getName") {
        next_int(id);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::NullString;
        } else {
            return api_get_output(misc_get_entity_name(entity), false) + '\n';
        }
    } else if (cmd == "world.getEntities") {
        next_int(type);
        if (api_compat_mode) {
            api_convert_to_mcpi_entity_type(type);
        }
        std::vector<std::string> result;
        for (Entity *entity : server->minecraft->level->entities) {
            int i = entity->getEntityTypeId();
            if (i > 0 && (type == no_entity_id || i == type)) {
                result.push_back(get_entity_message(server, entity));
            }
        }
        return api_join_outputs(result, list_separator);
    } else if (cmd == "world.removeEntity") {
        next_int(id);
        Entity *entity = server->minecraft->level->getEntity(id);
        int result = 0;
        if (entity != nullptr && !entity->isPlayer()) {
            entity->remove();
            result++;
        }
        return std::to_string(result) + '\n';
    } else if (cmd == "world.removeEntities") {
        next_int(type);
        if (api_compat_mode) {
            api_convert_to_mcpi_entity_type(type);
        }
        int removed = 0;
        for (Entity *entity : server->minecraft->level->entities) {
            int i = entity->getEntityTypeId();
            if (i > 0 && (type == no_entity_id || i == type)) {
                entity->remove();
                removed++;
            }
        }
        return std::to_string(removed) + '\n';
    } else if (cmd == "events.chat.posts") {
        return api_get_chat_events(server, client, std::nullopt);
    } else if (cmd == "entity.events.chat.posts") {
        next_int(id);
        return api_get_chat_events(server, client, id);
    } else if (cmd == "events.block.hits") {
        return api_get_block_hit_events(server, client, std::nullopt);
    } else if (cmd == "entity.events.block.hits") {
        next_int(id);
        return api_get_block_hit_events(server, client, id);
    } else if (cmd == "events.projectile.hits") {
        return api_get_projectile_events(server, client, std::nullopt);
    } else if (cmd == "entity.events.projectile.hits") {
        next_int(id);
        return api_get_projectile_events(server, client, id);
    } else if (cmd == "entity.setDirection") {
        next_int(id);
        next_float(x);
        next_float(y);
        next_float(z);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
        } else {
            set_dir(entity, x, y, z);
        }
        return CommandServer::NullString;
    } else if (cmd == "entity.getDirection") {
        next_int(id);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        } else {
            Vec3 vec = get_dir(entity);
            return api_join_outputs({std::to_string(vec.x), std::to_string(vec.y), std::to_string(vec.z)}, arg_separator);
        }
    } else if (cmd == "entity.setRotation") {
        next_int(id);
        next_float(yaw);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
        } else {
            entity->yaw = yaw;
        }
        return CommandServer::NullString;
    } else if (cmd == "entity.setPitch") {
        next_int(id);
        next_float(pitch);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
        } else {
            entity->pitch = pitch;
        }
        return CommandServer::NullString;
    } else if (cmd == "entity.getRotation") {
        next_int(id);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        } else {
            return std::to_string(entity->yaw) + '\n';
        }
    } else if (cmd == "entity.getPitch") {
        next_int(id);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        } else {
            return std::to_string(entity->pitch) + '\n';
        }
    } else if (cmd == "entity.getEntities") {
        // Parse
        next_int(id);
        next_int(dist);
        next_int(type);
        Entity *src = server->minecraft->level->getEntity(id);
        if (src == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        }
        // Run
        std::vector<std::string> result;
        for (Entity *entity : server->minecraft->level->entities) {
            int i = entity->getEntityTypeId();
            if (i > 0 && (type == no_entity_id || i == type) && distance_between(src, entity) < dist) {
                result.push_back(get_entity_message(server, entity));
            }
        }
        return api_join_outputs(result, list_separator);
    } else if (cmd == "entity.removeEntities") {
        // Parse
        next_int(id);
        next_int(dist);
        next_int(type);
        Entity *src = server->minecraft->level->getEntity(id);
        if (src == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        }
        // Run
        int removed = 0;
        for (Entity *entity : server->minecraft->level->entities) {
            int i = entity->getEntityTypeId();
            if (i > 0 && (type == no_entity_id || i == type) && distance_between(src, entity) < dist) {
                entity->remove();
                removed++;
            }
        }
        return std::to_string(removed) + '\n';
    } else if (cmd == "world.setSign") {
        // Parse
        next_int(x);
        next_int(y);
        next_int(z);
        next_int(id);
        next_int(data);
        // Translate
        server->pos_translator.from(x, y, z);
        // Set Block
        server->minecraft->level->setTileAndData(x, y, z, id, data);
        // Set Sign Data
        SignTileEntity *sign = get_sign(server, x, y, z);
        if (sign != nullptr) {
#define next_sign_line(i) \
next_string(line_##i, false); \
sign->lines[i] = get_input(line_##i); \
(void) 0
            next_sign_line(0);
            next_sign_line(1);
            next_sign_line(2);
            next_sign_line(3);
#undef next_sign_line
            // Send Update Packet
            sign->setChanged();
            Packet *packet = sign->getUpdatePacket();
            server->minecraft->rak_net_instance->send(*packet);
        }
        return CommandServer::NullString;
    } else if (cmd == "world.getSign") {
        // Parse
        next_int(x);
        next_int(y);
        next_int(z);
        // Translate
        server->pos_translator.from(x, y, z);
        // Read
        SignTileEntity *sign = get_sign(server, x, y, z);
        if (sign == nullptr) {
            return CommandServer::Fail;
        }
        std::vector<std::string> pieces;
        for (const std::string &line : sign->lines) {
            pieces.push_back(api_get_output(line, false));
        }
        return api_join_outputs(pieces, list_separator);
    } else if (cmd == "world.spawnEntity") {
        // Parse
        next_float(x);
        next_float(y);
        next_float(z);
        next_int(type);
        // Translate
        x -= server->pos_translator.x;
        y -= server->pos_translator.y;
        z -= server->pos_translator.z;
        if (api_compat_mode) {
            api_convert_to_mcpi_entity_type(type);
        }
        // Spawn
        Entity *entity = misc_make_entity_from_id(server->minecraft->level, type);
        if (entity == nullptr) {
            return CommandServer::Fail;
        }
        entity->moveTo(x, y, z, 0, 0);
        server->minecraft->level->addEntity(entity);
        return std::to_string(entity->id) + '\n';
    } else if (cmd == "world.getEntityTypes") {
        std::vector<std::string> result;
        for (const std::pair<const EntityType, std::pair<std::string, std::string>> &i : misc_get_entity_type_names()) {
            int id = static_cast<int>(i.first);
            if (api_compat_mode) {
                api_convert_to_rj_entity_type(id);
            }
            result.push_back(api_join_outputs({std::to_string(id), api_get_output(i.second.second, true)}, arg_separator));
        }
        return api_join_outputs(result, list_separator);
    } else if (cmd == "entity.setAbsPos") {
        next_int(id);
        next_float(x);
        next_float(y);
        next_float(z);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        }
        entity->moveTo(x, y, z, entity->yaw, entity->pitch);
        return CommandServer::NullString;
    } else if (cmd == "entity.getAbsPos") {
        next_int(id);
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        }
        return api_join_outputs({std::to_string(entity->x), std::to_string(entity->y), std::to_string(entity->z)}, arg_separator);
    } else if (cmd == "entity.events.clear") {
        next_int(id);
        api_clear_events(client, id);
        return CommandServer::NullString;
    } else if (cmd == "events.clear") {
        api_clear_events(client);
        return CommandServer::NullString;
    } else if (cmd == "reborn.disableCompatMode") {
        api_compat_mode = false;
        return std::string(reborn_get_version()) + '\n';
    } else if (cmd == "reborn.enableCompatMode") {
        api_compat_mode = true;
        return CommandServer::NullString;
    } else {
        // Call Original Method
        return old(server, client, command);
    }
}

// Init
void init_api() {
    if (feature_has("Implement RaspberryJuice API", server_enabled)) {
        overwrite_calls(CommandServer_parse, CommandServer_parse_injection);
        _init_api_events();
    }
}
