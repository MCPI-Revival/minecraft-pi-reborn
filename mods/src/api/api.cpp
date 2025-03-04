#include <cmath>
#include <sstream>

#include <libreborn/patch.h>
#include <libreborn/config.h>

#include <mods/api/api.h>
#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/feature/feature.h>

#include "internal.h"

// Force Semicolon After Calling Function Macro
#define force_semicolon() (void) 0

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
    server->pos_translator.from_int(start_x, start_y, start_z);
    server->pos_translator.from_int(end_x, end_y, end_z);

    // Swap If Needed
#define swap_if_needed(axis) \
    if (end_##axis < start_##axis) { \
        std::swap(start_##axis, end_##axis); \
    } \
    force_semicolon()
    swap_if_needed(x);
    swap_if_needed(y);
    swap_if_needed(z);
#undef swap_if_needed

    // Get
    std::vector<std::string> ret;
    for (int x = start_x; x <= end_x; x++) {
        for (int y = start_y; y <= end_y; y++) {
            for (int z = start_z; z <= end_z; z++) {
                std::vector<std::string> pieces;
                pieces.push_back(std::to_string(server->minecraft->level->getTile(x, y, z)));
                if (!api_compat_mode) {
                    pieces.push_back(std::to_string(server->minecraft->level->getData(x, y, z)));
                }
                ret.push_back(api_join_outputs(pieces, arg_separator));
            }
        }
    }
    // Return
    return api_join_outputs(ret, api_compat_mode ? arg_separator : list_separator);
}

// Properly Teleport Players
static void update_player_position(const Entity *entity) {
    if (entity->vtable == (Entity_vtable *) ServerPlayer_vtable::base) {
        const ServerPlayer *player = (ServerPlayer *) entity;
        MovePlayerPacket *packet = MovePlayerPacket::allocate();
        ((Packet *) packet)->constructor();
        packet->vtable = MovePlayerPacket_vtable::base;
        packet->x = player->x;
        packet->y = player->y - player->height_offset;
        packet->z = player->z;
        packet->yaw = player->yaw;
        packet->pitch = player->pitch;
        packet->entity_id = player->id;
        player->minecraft->rak_net_instance->send(*(Packet *) packet);
        packet->destructor_deleting();
    }
}
static void Entity_moveTo_injection(Entity *self, const float x, const float y, const float z, const float yaw, const float pitch) {
    self->moveTo(x, y, z, yaw, pitch);
    update_player_position(self);
}
static void ClientSideNetworkHandler_handle_MovePlayerPacket_injection(ClientSideNetworkHandler_handle_MovePlayerPacket_t original, ClientSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, MovePlayerPacket *packet) {
    if (self->level) {
        Entity *entity = self->level->getEntity(packet->entity_id);
        if (entity) {
            if (entity == (Entity *) self->minecraft->player) {
                // Just Teleport
                entity->moveTo(packet->x, packet->y, packet->z, packet->yaw, packet->pitch);
            } else {
                // Call Original Method
                original(self, rak_net_guid, packet);
            }
        }
    }
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
    update_player_position(entity);
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
static std::string get_entity_message(CommandServer *server, Entity *entity) {
    std::vector<std::string> pieces;
    // ID
    pieces.push_back(std::to_string(entity->id));
    // Type
    int type = entity->getEntityTypeId();
    api_convert_to_outside_entity_type(type);
    pieces.push_back(std::to_string(type));
    if (api_compat_mode) {
        pieces.push_back(api_get_output(misc_get_entity_type_name(entity).second, true));
    }
    // XYZ
    float x = entity->x;
    float y = entity->y - entity->height_offset;
    float z = entity->z;
    server->pos_translator.to_float(x, y, z);
    pieces.push_back(std::to_string(x));
    pieces.push_back(std::to_string(y));
    pieces.push_back(std::to_string(z));
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

// Get Sign Tile Entity
static SignTileEntity *get_sign(const CommandServer *server, const int x, const int y, const int z) {
    TileEntity *sign = server->minecraft->level->getTileEntity(x, y, z);
    if (sign != nullptr && sign->type == 4) {
        return (SignTileEntity *) sign;
    } else {
        return nullptr;
    }
}

// Check If Entity Is Selected
static bool is_entity_selected(Entity *entity, const int target_type) {
    const int type = entity->getEntityTypeId();
    return type > 0 && (target_type == no_entity_id || target_type == type);
}

// Read Arguments
#define next_string(out, required) \
    std::string out; \
    const bool out##_missing = !std::getline(args, out, arg_separator); \
    if (out##_missing && (required)) { \
        return CommandServer::Fail; \
    } \
    force_semicolon()
#define next_number(out, type, func) \
    next_string(out##_str, true); \
    type out; \
    try { \
        out = func(out##_str); \
    } catch (...) { \
        return CommandServer::Fail; \
    } \
    force_semicolon()
#define next_int(out) next_number(out, int, std::stoi)
#define next_float(out) next_number(out, float, std::stof)
// Parse API Commands
#define package_str(name) (#name ".")
static bool _package(std::string &cmd, const std::string &package) {
    if (cmd.starts_with(package)) {
        cmd = cmd.substr(package.size());
        return true;
    } else {
        return false;
    }
}
#define package(name) if (_package(cmd, package_str(name)))
#define command(name) if (cmd == #name)
#define passthrough(name) \
    command(name) { \
        api_suppress_chat_events = client.sock >= 0; /* Suppress If Real API Client */ \
        std::string ret = original(server, client, command); \
        api_suppress_chat_events = false; \
        return ret; \
    } \
    force_semicolon()
static std::string CommandServer_parse_injection(CommandServer_parse_t original, CommandServer *server, ConnectedClient &client, const std::string &command) {
    // Parse Command
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

    // Redirect Player Package To Entity
    package(player) {
        // The One Exception
        passthrough(setting);
        // Redirect All Other Commands
        LocalPlayer *player = server->minecraft->player;
        if (player) {
            cmd = package_str(entity) + cmd;
            args_str = std::to_string(player->id) + arg_separator + args_str;
        } else {
            return CommandServer::Fail;
        }
    }

    // Read Arguments
    std::stringstream args(args_str);

    // Manipulate The Level
    package(world) {
        // Vanilla Commands
        passthrough(setBlock);
        passthrough(getBlock);
        passthrough(getBlockWithData);
        passthrough(setBlocks);
        passthrough(getHeight);
        passthrough(getPlayerIds);
        passthrough(checkpoint.save);
        passthrough(checkpoint.restore);
        passthrough(setting);

        // Get Region Of Tiles
        command(getBlocks) {
            // Parse
            next_int(x0);
            next_int(y0);
            next_int(z0);
            next_int(x1);
            next_int(y1);
            next_int(z1);
            // Get The Blocks
            return get_blocks(server, Vec3{(float) x0, (float) y0, (float) z0}, Vec3{(float) x1, (float) y1, (float) z1});
        }

        // Get Player With Username
        command(getPlayerId) {
            // Parse
            next_string(input, true);
            // Search
            std::string username = api_get_input(input);
            for (Player *player : server->minecraft->level->players) {
                if (misc_get_player_username_utf(player) == username) {
                    // Found
                    return std::to_string(player->id) + "\n";
                }
            }
            return CommandServer::Fail;
        }

        // Get/Remove Entities
        command(getEntities) {
            // Parse
            next_int(type);
            api_convert_to_mcpi_entity_type(type);
            // Search
            std::vector<std::string> result;
            for (Entity *entity : server->minecraft->level->entities) {
                if (is_entity_selected(entity, type)) {
                    result.push_back(get_entity_message(server, entity));
                }
            }
            return api_join_outputs(result, list_separator);
        }
        command(removeEntity) {
            // Parse
            next_int(id);
            // Remove
            Entity *entity = server->minecraft->level->getEntity(id);
            int result = 0;
            if (entity != nullptr && !entity->isPlayer()) {
                entity->remove();
                result++;
            }
            return std::to_string(result) + '\n';
        }
        command(removeEntities) {
            // Parse
            next_int(type);
            api_convert_to_mcpi_entity_type(type);
            // Remove
            int removed = 0;
            for (Entity *entity : server->minecraft->level->entities) {
                if (is_entity_selected(entity, type)) {
                    entity->remove();
                    removed++;
                }
            }
            return std::to_string(removed) + '\n';
        }

        // Spawn Entity
        command(spawnEntity) {
            // Parse
            next_float(x);
            next_float(y);
            next_float(z);
            next_int(type);
            // Translate
            server->pos_translator.from_float(x, y, z);
            if (api_compat_mode) {
                // RJ Only Accepted Integers
                x = float(int(x));
                y = float(int(y));
                z = float(int(z));
            }
            api_convert_to_mcpi_entity_type(type);
            // Spawn
            Entity *entity = misc_make_entity_from_id(server->minecraft->level, type, x, y, z);
            if (entity == nullptr) {
                return CommandServer::Fail;
            }
            return std::to_string(entity->id) + '\n';
        }

        // Get All Valid Entity Types
        command(getEntityTypes) {
            std::vector<std::string> result;
            for (const std::pair<const EntityType, std::pair<std::string, std::string>> &i : misc_get_entity_type_names()) {
                int id = static_cast<int>(i.first);
                api_convert_to_outside_entity_type(id);
                result.push_back(api_join_outputs({std::to_string(id), api_get_output(i.second.second, true)}, arg_separator));
            }
            return api_join_outputs(result, list_separator);
        }

        // Signs
        command(setSign) {
            // Parse
            next_int(x);
            next_int(y);
            next_int(z);
            next_int(id);
            next_int(data);
            // Translate
            server->pos_translator.from_int(x, y, z);
            // Set Block
            server->minecraft->level->setTileAndData(x, y, z, id, data);
            // Set Sign Data
            SignTileEntity *sign = get_sign(server, x, y, z);
            if (sign != nullptr) {
#define next_sign_line(i) \
    next_string(line_##i, false); \
    if (!api_compat_mode || !line_##i##_missing) { \
        sign->lines[i] = api_get_input(line_##i); \
    } \
    force_semicolon()
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
        }
        command(getSign) {
            // Parse
            next_int(x);
            next_int(y);
            next_int(z);
            // Translate
            server->pos_translator.from_int(x, y, z);
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
        }

        // Level Information
        command(getSeed) {
            return std::to_string(server->minecraft->level->data.seed) + '\n';
        }
        command(getGameMode) {
            return std::to_string(server->minecraft->isCreativeMode()) + '\n';
        }
    }

    // Manipulate An Individual Entity
    package(entity) {
        // Vanilla Commands
        passthrough(setTile);
        passthrough(getTile);
        passthrough(setPos);
        passthrough(getPos);

        // Common Code For New Commands
        next_int(id);
#define _get_entity() Entity *entity = server->minecraft->level->getEntity(id)
#define get_entity(ret_on_invalid) \
    _get_entity(); \
    if (entity == nullptr) { \
        /* set* Commands Usually Return Null */ \
        /* get* Commands Usually Return Fail */ \
        return CommandServer::ret_on_invalid; \
    } \
    force_semicolon()

        // Name/Type/ID
        command(getName) {
            get_entity(NullString); // Matching RJ Behavior, Even Though It is Dumb
            return api_get_output(misc_get_entity_name(entity), false) + '\n';
        }
        command(getType) {
            get_entity(Fail);
            int type = entity->getEntityTypeId();
            api_convert_to_outside_entity_type(type);
            return std::to_string(type) + '\n';
        }
        command(getId) {
            // Parse
            get_entity(Fail);
            // Check If Entity Exists
            return std::to_string(entity->id) + '\n'; // I Promise This Is Useful
        }

        // Get/Remove Nearby Entities
        package(getEntities) {
            // Parse
            _get_entity(); // Matching RJ Behavior, Even Though It is Dumb
            next_int(dist);
            next_int(type);
            api_convert_to_mcpi_entity_type(type);
            // Run
            std::vector<std::string> result;
            for (Entity *other : server->minecraft->level->entities) {
                if (is_entity_selected(other, type) && distance_between(entity, other) <= dist) {
                    result.push_back(get_entity_message(server, other));
                }
            }
            return api_join_outputs(result, list_separator);
        }
        command(removeEntities) {
            // Parse
            _get_entity(); // Matching RJ Behavior, Even Though It is Dumb
            next_int(dist);
            next_int(type);
            api_convert_to_mcpi_entity_type(type);
            // Run
            int removed = 0;
            for (Entity *other : server->minecraft->level->entities) {
                if (is_entity_selected(other, type) && distance_between(entity, other) <= dist) {
                    other->remove();
                    removed++;
                }
            }
            return std::to_string(removed) + '\n';
        }

        // Get/Set Rotation
        command(setDirection) {
            // Parse
            get_entity(NullString);
            next_float(x);
            next_float(y);
            next_float(z);
            // Set
            set_dir(entity, x, y, z);
            return CommandServer::NullString;
        }
        command(getDirection) {
            // Parse
            get_entity(Fail);
            // Get
            Vec3 vec = get_dir(entity);
            return api_join_outputs({std::to_string(vec.x), std::to_string(vec.y), std::to_string(vec.z)}, arg_separator);
        }
        command(setRotation) {
            // Parse
            get_entity(NullString);
            next_float(yaw);
            // Set
            entity->yaw = yaw;
            update_player_position(entity);
            return CommandServer::NullString;
        }
        command(getRotation) {
            // Parse
            get_entity(Fail);
            // Get
            return std::to_string(entity->yaw) + '\n';
        }
        command(setPitch) {
            // Parse
            get_entity(NullString);
            next_float(pitch);
            // Set
            entity->pitch = pitch;
            update_player_position(entity);
            return CommandServer::NullString;
        }
        command(getPitch) {
            // Parse
            get_entity(Fail);
            // Get
            return std::to_string(entity->pitch) + '\n';
        }

        // Get/Set Absolute Position
        command(setAbsPos) {
            // Parse
            get_entity(NullString);
            next_float(x);
            next_float(y);
            next_float(z);
            // Set
            Entity_moveTo_injection(entity, x, y, z, entity->yaw, entity->pitch);
            return CommandServer::NullString;
        }
        command(getAbsPos) {
            // Parse
            get_entity(Fail);
            // Get
            return api_join_outputs({std::to_string(entity->x), std::to_string(entity->y - entity->height_offset), std::to_string(entity->z)}, arg_separator);
        }

        // Get/Set Velocity
        command(setVelocity) {
            // Get Entity
            get_entity(NullString);
            // Set Velocity
            SetEntityMotionPacket *packet = SetEntityMotionPacket::allocate();
            ((Packet *) packet)->constructor();
            packet->vtable = SetEntityMotionPacket_vtable::base;
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
            get_entity(Fail);
            // Get
            return api_join_outputs({std::to_string(entity->velocity_x), std::to_string(entity->velocity_y), std::to_string(entity->velocity_z)}, arg_separator);
        }
#undef get_entity
#undef _get_entity

        // Handle Events
        package(events) {
            return api_handle_event_command(server, client, cmd, id);
        }
    }

    // Chat
    package(chat) {
        passthrough(post);
    }

    // Manipulate The Camera
    package(camera) {
        passthrough(mode.setFixed);
        passthrough(mode.setNormal);
        passthrough(mode.setFollow);
        passthrough(setPos);
    }

    // Handle Events
    package(events) {
        return api_handle_event_command(server, client, cmd, std::nullopt);
    }

    // Compatibility Mode
    package(reborn) {
        // Disable Compatibility Mode
        command(disableCompatMode) {
            api_compat_mode = false;
            return std::string(reborn_get_version()) + '\n';
        }
        // Re-Enable Compatibility Mode
        command(enableCompatMode) {
            api_compat_mode = true;
            return CommandServer::NullString;
        }
    }

    // Invalid Command
    return CommandServer::Fail;
}

// Fix HUD Spectating Other Players
template <typename... Args>
static void ItemInHandRenderer_render_injection(const std::function<void(ItemInHandRenderer *, Args...)> &original, ItemInHandRenderer *self, Args... args) {
    // "Fix" Current Player
    LocalPlayer *&player = self->minecraft->player;
    LocalPlayer *old_player = player;
    Mob *camera = self->minecraft->camera;
    if (camera && camera->isPlayer()) {
        player = (LocalPlayer *) camera;
    }
    // Call Original Method
    original(self, std::forward<Args>(args)...);
    // Revert "Fix"
    player = old_player;
}

// Fix Crash When Camera Entity Is Removed
static void LevelRenderer_entityRemoved_injection(LevelRenderer *self, Entity *entity) {
    // Call Original Method
    LevelListener_entityRemoved->get(false)((LevelListener *) self, entity);
    // Fix Camera
    Minecraft *minecraft = self->minecraft;
    if ((Entity *) minecraft->camera == entity) {
        minecraft->camera = (Mob *) minecraft->player;
    }
}

// Close Sockets
static void CommandServer__close_injection(CommandServer__close_t original, CommandServer *self) {
    // Close
    for (const ConnectedClient &client : self->clients) {
        close(client.sock);
    }
    self->clients.clear();
    // Call Original Method
    original(self);
}
static void Minecraft_leaveGame_injection(Minecraft_leaveGame_t original, Minecraft *self, const bool save_remote_level) {
    // Destroy Server
    CommandServer *&server = self->command_server;
    if (server) {
        server->destructor(0);
        server = nullptr;
    }
    // Call Original Method
    original(self, save_remote_level);
}

// Init
void init_api() {
    if (feature_has("Implement RaspberryJuice API", server_enabled)) {
        overwrite_calls(CommandServer_parse, CommandServer_parse_injection);
        _init_api_events();
        // Fix Teleporting Players
        overwrite_calls(ClientSideNetworkHandler_handle_MovePlayerPacket, ClientSideNetworkHandler_handle_MovePlayerPacket_injection);
        overwrite_call((void *) 0x6b6e8, Entity_moveTo, Entity_moveTo_injection);
    }
    // Bug Fixes
    if (feature_has("Fix HUD When Spectating Other Players", server_enabled)) {
        overwrite_calls(ItemInHandRenderer_render, ItemInHandRenderer_render_injection<float>);
        overwrite_calls(ItemInHandRenderer_renderScreenEffect, ItemInHandRenderer_render_injection<float>);
        overwrite_calls(ItemInHandRenderer_tick, ItemInHandRenderer_render_injection<>);
    }
    if (feature_has("Fix Crash When Spectated Entity Is Removed", server_enabled)) {
        patch_vtable(LevelRenderer_entityRemoved, LevelRenderer_entityRemoved_injection);
    }
    if (feature_has("Correctly Close API Sockets", server_enabled)) {
        overwrite_calls(CommandServer__close, CommandServer__close_injection);
        overwrite_calls(Minecraft_leaveGame, Minecraft_leaveGame_injection);
    }
}
