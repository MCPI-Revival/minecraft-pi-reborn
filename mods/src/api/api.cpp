#include <cmath>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <optional>
#include <ranges>

#include <libreborn/log.h>
#include <libreborn/util/string.h>
#include <libreborn/patch.h>
#include <libreborn/config.h>
#include <symbols/minecraft.h>

#include <mods/api/api.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/chat/chat.h>
#include <mods/feature/feature.h>

// Compatibility Mode
static bool compat_mode = true;

// Argument Separator
static constexpr char arg_separator = ',';
static constexpr char list_separator = '|';

// Read String Input
static std::string get_input(std::string message) {
    // Decode
    if (!compat_mode) {
        message = misc_base64_decode(message);
    }
    // Convert To CP-437
    return to_cp437(message);
}
// Output String
static std::string get_output(std::string message, const bool replace_comma = false) {
    if (compat_mode) {
        // Output In Plaintext For RJ Compatibility
        std::ranges::replace(message, list_separator, '\\');
        if (replace_comma) {
            std::ranges::replace(message, arg_separator, '.');
        }
    } else {
        message = misc_base64_encode(message);
    }
    // Convert To Unicode
    return from_cp437(message);
}

// Join Strings Into Output
static std::string join_outputs(const std::vector<std::string> &pieces, const char separator = arg_separator) {
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
    int startx = int(start.x);
    int starty = int(start.y);
    int startz = int(start.z);
    // End Coordinate
    int endx = int(end.x);
    int endy = int(end.y);
    int endz = int(end.z);

    // Apply Offset
    server->pos_translator.from(startx, starty, startz);
    server->pos_translator.from(endx, endy, endz);

    // Swap If Needed
    if (endx < startx) {
        std::swap(startx, endx);
    }
    if (endy < starty) {
        std::swap(starty, endy);
    }
    if (endz < startz) {
        std::swap(startz, endz);
    }

    // Get
    std::vector<std::string> ret;
    for (int x = startx; x <= endx; x++) {
        for (int y = starty; y <= endy; y++) {
            for (int z = startz; z <= endz; z++) {
                ret.push_back(std::to_string(server->minecraft->level->getTile(x, y, z)));
            }
        }
    }
    // Return
    return join_outputs(ret);
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
std::unordered_map<int, EntityType> modern_entity_id_mapping = {
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
static void convert_to_rj_entity_type(int &type) {
    for (const std::pair<const int, EntityType> &pair : modern_entity_id_mapping) {
        if (static_cast<int>(pair.second) == type) {
            type = pair.first;
        }
    }
}
static void convert_to_mcpi_entity_type(int &type) {
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
    if (compat_mode) {
        convert_to_rj_entity_type(type);
    }
    // Return
    return join_outputs({
        // ID
        std::to_string(entity->id),
        // Type
        std::to_string(type),
        // Name
        get_output(misc_get_entity_type_name(entity).second, true),
        // X
        std::to_string(x),
        // Y
        std::to_string(y),
        // X
        std::to_string(z)
    });
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

// Projectile Event
struct ProjectileHitEvent {
    int x;
    int y;
    int z;
    std::string owner = "";
    int owner_id = 0;
    int target_id = 0;
};
static std::string event_to_string(CommandServer *server, const ProjectileHitEvent &e) {
    // Offset Position
    float nx = float(e.x);
    float ny = float(e.y);
    float nz = float(e.z);
    server->pos_translator.to(nx, ny, nz);
    // Get Outputs
    std::vector pieces = {
        // Position
        std::to_string(int(nx)),
        std::to_string(int(ny)),
        std::to_string(int(nz))
    };
    // Needed For Compatibility
    if (compat_mode) {
        pieces.push_back("1");
    }
    // Owner
    if (compat_mode) {
        pieces.push_back(get_output(e.owner, true));
    } else {
        pieces.push_back(std::to_string(e.owner_id));
        pieces.push_back(std::to_string(e.target_id));
    }
    // Return
    return join_outputs(pieces);
}

// Chat Message Event
struct ChatEvent {
    std::string message = "";
    int owner_id = 0;
};
static std::string event_to_string(__attribute__((unused)) CommandServer *server, const ChatEvent &e) {
    return join_outputs({
        std::to_string(e.owner_id),
        get_output(e.message, true),
    });
}

// Block Hit Event
static std::string event_to_string(CommandServer *server, const TileEvent &e) {
    // Offset Coordinates
    float x = float(e.x);
    float y = float(e.y);
    float z = float(e.z);
    server->pos_translator.to(x, y, z);
    // Output
    return join_outputs({
        // Position
        std::to_string(int(x)),
        std::to_string(int(y)),
        std::to_string(int(z)),
        // Face
        std::to_string(e.face),
        // Entity ID
        std::to_string(e.owner_id)
    });
}

// Track Event Queues Per Client
struct ExtraClientData {
    std::vector<ProjectileHitEvent> projectile_events;
    std::vector<ChatEvent> chat_events;
    std::vector<TileEvent> block_hit_events;
};
static std::unordered_map<int, ExtraClientData> extra_client_data;
static bool CommandServer__updateAccept_setSocketBlocking_injection(const int fd, const bool param_1) {
    // Client Was Created
    extra_client_data[fd] = ExtraClientData();
    return CommandServer::setSocketBlocking(fd, param_1);
}
static bool CommandServer__updateClient_injection(CommandServer__updateClient_t original, CommandServer *self, ConnectedClient &client) {
    const bool ret = original(self, client);
    if (!ret) {
        // Client Disconnected
        extra_client_data.erase(client.sock);
    }
    return ret;
}

// Clear All Events
static void clear_events(const ConnectedClient &client) {
    ExtraClientData &data = extra_client_data[client.sock];
    if (!compat_mode) {
        // Match RJ Bug
        data.projectile_events.clear();
    }
    data.chat_events.clear();
    data.block_hit_events.clear();
}

// Clear Events Produced By Given Entity
template <typename T>
static void clear_events(std::vector<T> &data, const int id) {
    std::ranges::remove_if(data, [&id](const T &e) {
        return id == e.owner_id;
    });
}
static void clear_events(const ConnectedClient &client, const int id) {
    ExtraClientData &data = extra_client_data[client.sock];
    clear_events(data.block_hit_events, id);
    clear_events(data.chat_events, id);
    clear_events(data.projectile_events, id);
}

// Get Events In Queue
template <typename T>
static std::string get_events(CommandServer *server, std::vector<T> &queue, const std::optional<int> id) {
    std::vector<std::string> out;
    typename std::vector<T>::iterator it = queue.begin();
    while (it != queue.end()) {
        const T &e = *it;
        if (id.has_value() && id.value() != e.owner_id) {
            // Skip Event
            ++it;
            continue;
        }
        // Output
        out.push_back(event_to_string(server, e));
        // Erase
        it = queue.erase(it);
    }
    return join_outputs(out, list_separator);
}

// Map RJ Entity IDs To MCPI IDs
static constexpr int no_entity_id = -1;
static const std::string player_namespace = "player.";
#define API_WARN(format, ...) WARN("API: %s: " format, cmd.c_str(), ##__VA_ARGS__)
#define ENTITY_NOT_FOUND API_WARN("Entity Not Found: %i", id)
std::string CommandServer_parse_injection(CommandServer_parse_t old, CommandServer *server, ConnectedClient &client, const std::string &command) {
    size_t arg_start = command.find("(");
    if (arg_start == std::string::npos) {
        return CommandServer::Fail;
    }
    std::string cmd = command.substr(0, arg_start);
    // rfind() So ) In Input Does Not Break
    size_t cmd_end = command.rfind(")");
    if (cmd_end == std::string::npos) {
        return CommandServer::Fail;
    }
    std::string args = command.substr(arg_start + 1, cmd_end - arg_start - 1);

    // Redirect Player Namespace To The Entity One
    if (server->minecraft->player != nullptr) {
        if (cmd.starts_with(player_namespace) && cmd != "player.setting") {
            cmd = "entity." + cmd.substr(player_namespace.size());
            args = std::to_string(server->minecraft->player->id) + arg_separator + args;
        }
    }

    // And Now The Big If-Else Chain
    if (cmd == "world.getBlocks") {
        int x0, y0, z0, x1, y1, z1;
        if (sscanf(args.c_str(), "%d,%d,%d,%d,%d,%d", &x0, &y0, &z0, &x1, &y1, &z1) != 6) {
            return CommandServer::Fail;
        }
        // Get The Blocks
        return get_blocks(server, Vec3{(float) x0, (float) y0, (float) z0}, Vec3{(float) x1, (float) y1, (float) z1});
    } else if (cmd == "world.getPlayerId") {
        std::string username = get_input(args);
        for (Player *player : server->minecraft->level->players) {
            if (misc_get_player_username_utf(player) == username) {
                return std::to_string(player->id) + "\n";
            }
        }
        API_WARN("Player Not Found: %s", args.c_str());
        return CommandServer::Fail;
    } else if (cmd == "entity.getName") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::NullString;
        } else {
            return get_output(misc_get_entity_name(entity)) + '\n';
        }
    } else if (cmd == "world.getEntities") {
        int type;
        if (sscanf(args.c_str(), "%d", &type) != 1) {
            return CommandServer::Fail;
        }
        if (compat_mode) {
            convert_to_mcpi_entity_type(type);
        }
        std::vector<std::string> result;
        for (Entity *entity : server->minecraft->level->entities) {
            int i = entity->getEntityTypeId();
            if (i > 0 && (type == no_entity_id || i == type)) {
                result.push_back(get_entity_message(server, entity));
            }
        }
        return join_outputs(result, list_separator);
    } else if (cmd == "world.removeEntity") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        int result = 0;
        if (entity != nullptr && !entity->isPlayer()) {
            entity->remove();
            result++;
        }
        return std::to_string(result) + '\n';
    } else if (cmd == "world.removeEntities") {
        int type;
        if (sscanf(args.c_str(), "%d", &type) != 1) {
            return CommandServer::Fail;
        }
        if (compat_mode) {
            convert_to_mcpi_entity_type(type);
        }
        int removed = 0;
        for (Entity *entity : server->minecraft->level->entities) {
            int i = entity->getEntityTypeId();
            if (i > 0) {
                if (type == no_entity_id || i == type) {
                    entity->remove();
                    removed++;
                }
            }
        }
        return std::to_string(removed) + '\n';
    } else if (cmd == "events.chat.posts") {
        return get_events(server, extra_client_data[client.sock].chat_events, std::nullopt);
    } else if (cmd == "entity.events.chat.posts") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        return get_events(server, extra_client_data[client.sock].chat_events, id);
    } else if (cmd == "events.block.hits") {
        return get_events(server, extra_client_data[client.sock].block_hit_events, std::nullopt);
    } else if (cmd == "entity.events.block.hits") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        return get_events(server, extra_client_data[client.sock].block_hit_events, id);
    } else if (cmd == "events.projectile.hits") {
        return get_events(server, extra_client_data[client.sock].projectile_events, std::nullopt);
    } else if (cmd == "entity.events.projectile.hits") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        return get_events(server, extra_client_data[client.sock].projectile_events, id);
    } else if (cmd == "entity.setDirection") {
        int id;
        float x, y, z;
        if (sscanf(args.c_str(), "%d,%f,%f,%f", &id, &x, &y, &z) != 4) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
        } else {
            set_dir(entity, x, y, z);
        }
        return CommandServer::NullString;
    } else if (cmd == "entity.getDirection") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        } else {
            Vec3 vec = get_dir(entity);
            return join_outputs({std::to_string(vec.x), std::to_string(vec.y), std::to_string(vec.z)});
        }
    } else if (cmd == "entity.setRotation") {
        int id;
        float yaw;
        if (sscanf(args.c_str(), "%d,%f", &id, &yaw) != 2) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
        } else {
            entity->yaw = yaw;
        }
        return CommandServer::NullString;
    } else if (cmd == "entity.setPitch") {
        int id;
        float pitch;
        if (sscanf(args.c_str(), "%d,%f", &id, &pitch) != 2) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
        } else {
            entity->pitch = pitch;
        }
        return CommandServer::NullString;
    } else if (cmd == "entity.getRotation") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        } else {
            return std::to_string(entity->yaw) + '\n';
        }
    } else if (cmd == "entity.getPitch") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        } else {
            return std::to_string(entity->pitch) + '\n';
        }
    } else if (cmd == "entity.getEntities") {
        // Parse
        int dist, type;
        int id = 0;
        if (sscanf(args.c_str(), "%d,%d,%d", &id, &dist, &type) != 3) {
            return CommandServer::Fail;
        }
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
        return join_outputs(result, list_separator);
    } else if (cmd == "entity.removeEntities") {
        // Parse
        int dist, type;
        int id = 0;
        if (sscanf(args.c_str(), "%d,%d,%d", &id, &dist, &type) != 3) {
            return CommandServer::Fail;
        }
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
        int x, y, z, id, data;
        char l1[100], l2[100], l3[100], l4[100];
        int ret = sscanf(args.c_str(), "%d,%d,%d,%d,%d,%99[^,],%99[^,],%99[^,],%99s", &x, &y, &z, &id, &data, l1, l2, l3, l4);
        constexpr int minimal_arg_count = 5;
        constexpr int line_count = 4;
        if (ret < minimal_arg_count || ret > (minimal_arg_count + line_count)) {
            return CommandServer::Fail;
        }
        // Translate
        server->pos_translator.from(x, y, z);
        // Set Block
        server->minecraft->level->setTileAndData(x, y, z, id, data);
        // Set Sign Data
        if (ret == minimal_arg_count) {
            return CommandServer::NullString;
        }
        SignTileEntity *sign = (SignTileEntity *) server->minecraft->level->getTileEntity(x, y, z);
        if (sign == nullptr || sign->type != 4) {
            return CommandServer::NullString;
        }
        char *lines[line_count] = {l1, l2, l3, l4};
        for (int i = 0; i < line_count; i++) {
            if (ret > (i + minimal_arg_count)) {
                sign->lines[i] = get_input(lines[i]);
            }
        }
        // Send Update Packet
        sign->setChanged();
        Packet *packet = sign->getUpdatePacket();
        server->minecraft->rak_net_instance->send(*packet);
        return CommandServer::NullString;
    } else if (cmd == "world.spawnEntity") {
        // Parse
        float x, y, z;
        int id;
        if (sscanf(args.c_str(), "%f,%f,%f,%d", &x, &y, &z, &id) != 4) {
            return CommandServer::Fail;
        }
        // Translate
        x -= server->pos_translator.x;
        y -= server->pos_translator.y;
        z -= server->pos_translator.z;
        if (compat_mode) {
            convert_to_mcpi_entity_type(id);
        }
        // Spawn
        Entity *entity = misc_make_entity_from_id(server->minecraft->level, id);
        if (entity == nullptr) {
            return CommandServer::Fail;
        }
        entity->moveTo(x, y, z, 0, 0);
        server->minecraft->level->addEntity(entity);
        return std::to_string(entity->id) + '\n';
    } else if (cmd == "world.getEntityTypes") {
        std::vector<std::string> result;
        for (const std::pair<const EntityType, std::pair<std::string, std::string>> &i: misc_get_entity_type_names()) {
            int id = static_cast<int>(i.first);
            if (compat_mode) {
                convert_to_rj_entity_type(id);
            }
            result.push_back(join_outputs({std::to_string(id), i.second.second}));
        }
        return join_outputs(result, list_separator);
    } else if (cmd == "entity.setAbsPos") {
        int id;
        float x, y, z;
        if (sscanf(args.c_str(), "%d,%f,%f,%f", &id, &x, &y, &z) != 4) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        }
        entity->moveTo(x, y, z, entity->yaw, entity->pitch);
        return CommandServer::NullString;
    } else if (cmd == "entity.getAbsPos") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        Entity *entity = server->minecraft->level->getEntity(id);
        if (entity == nullptr) {
            ENTITY_NOT_FOUND;
            return CommandServer::Fail;
        }
        return join_outputs({std::to_string(entity->x), std::to_string(entity->y), std::to_string(entity->z)});
    } else if (cmd == "entity.events.clear") {
        int id;
        if (sscanf(args.c_str(), "%d", &id) != 1) {
            return CommandServer::Fail;
        }
        clear_events(client, id);
        return CommandServer::NullString;
    } else if (cmd == "events.clear") {
        clear_events(client);
        return CommandServer::NullString;
    } else if (cmd == "reborn.disableCompatMode") {
        compat_mode = false;
        return std::string(reborn_get_version()) + '\n';
    } else if (cmd == "reborn.enableCompatMode") {
        compat_mode = true;
        return CommandServer::NullString;
    } else {
        // Call Original Method
        return old(server, client, command);
    }
}

// Push Event To Queue
template <typename T>
static void push_event(std::vector<T> ExtraClientData::*ptr, const T e) {
    for (ExtraClientData &data : extra_client_data | std::views::values) {
        (data.*ptr).push_back(e);
    }
}

// Arrow Entity Hit
static Entity *shooter = nullptr;
static HitResult *Arrow_tick_HitResult_constructor_injection(HitResult *self, Entity *target) {
    // Original
    self->constructor(target);
    // Add event
    if (shooter && shooter->isPlayer()) {
        push_event(&ExtraClientData::projectile_events, {
            .x = (int) target->x,
            .y = (int) target->y,
            .z = (int) target->z,
            .owner = ((Player *) shooter)->username,
            .owner_id = shooter->id,
            .target_id = target->id
        });
    }
    return self;
}

// Arrow block hit
static void Arrow_tick_injection(Arrow_tick_t old, Arrow *self) {
    const int oldFlightTime = self->flight_time;
    shooter = self->level->getEntity(self->getAuxData());
    old(self);
    if (!self->pending_removal && self->grounded && oldFlightTime != self->flight_time) {
        if (shooter && shooter->isPlayer()) {
            // Hit! Get the data
            push_event(&ExtraClientData::projectile_events, {
                .x = self->hit_x,
                .y = self->hit_y,
                .z = self->hit_z,
                .owner = ((Player *) shooter)->username,
                .owner_id = shooter->id,
                .target_id = 0
            });
        }
    }
}

// Throwable Hits
static void Throwable_tick_Throwable_onHit_injection(Throwable *self, HitResult *res) {
    if (res == nullptr || res->type == 2) {
        return self->onHit(res);
    }
    Entity *thrower = self->level->getEntity(self->getAuxData());
    if (thrower == nullptr || !thrower->isPlayer()) {
        return self->onHit(res);
    }
    ProjectileHitEvent event;
    if (res->type == 1 && res->entity) {
        // Entity
        event = {
            .x = (int) res->exact.x,
            .y = (int) res->exact.y,
            .z = (int) res->exact.z,
            .owner = ((Player *) thrower)->username,
            .owner_id = thrower->id,
            .target_id = res->entity->id
        };
    } else {
        // Tile
        event = {
            .x = res->x,
            .y = res->y,
            .z = res->z,
            .owner = ((Player *) thrower)->username,
            .owner_id = thrower->id,
            .target_id = 0
        };
    }
    push_event(&ExtraClientData::projectile_events, event);
    self->onHit(res);
}

// Chat Events
static void Gui_addMessage_injection(Gui_addMessage_t original, Gui *gui, const std::string &text) {
    static bool recursing = false;
    if (recursing) {
        original(gui, text);
    } else {
        if (!chat_is_sending()) {
            api_add_chat_event(nullptr, text);
        }
        recursing = true;
        original(gui, text);
        recursing = false;
    }
}
static bool enabled = false;
void api_add_chat_event(const Player *sender, const std::string &message) {
    if (!enabled || (!sender && compat_mode)) {
        return;
    }
    push_event(&ExtraClientData::chat_events, ChatEvent{message, sender ? sender->id : no_entity_id});
}

// Init
void init_api() {
    if (feature_has("Implement RaspberryJuice API", server_enabled)) {
        enabled = true;
        overwrite_calls(CommandServer_parse, CommandServer_parse_injection);
        overwrite_calls(Arrow_tick, Arrow_tick_injection);
        overwrite_call((void *) 0x8b1e8, HitResult_constructor, Arrow_tick_HitResult_constructor_injection);
        overwrite_call((void *) 0x8c5a4, Throwable_onHit, Throwable_tick_Throwable_onHit_injection);
        overwrite_calls(Gui_addMessage, Gui_addMessage_injection);
        overwrite_call((void *) 0x6bd78, CommandServer_setSocketBlocking, CommandServer__updateAccept_setSocketBlocking_injection);
        overwrite_calls(CommandServer__updateClient, CommandServer__updateClient_injection);
    }
}
