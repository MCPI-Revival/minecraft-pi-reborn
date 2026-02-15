#include "internal.h"

#include <libreborn/util/string.h>
#include <libreborn/log.h>

#include <symbols/CommandServer.h>
#include <symbols/Vec3.h>
#include <symbols/Minecraft.h>
#include <symbols/Level.h>
#include <symbols/Player.h>
#include <symbols/Entity.h>
#include <symbols/TileEntity.h>
#include <symbols/Item.h>
#include <symbols/ItemEntity.h>
#include <symbols/SignTileEntity.h>
#include <symbols/RakNetInstance.h>

#include <mods/misc/misc.h>

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
    const size_t expected_size = (end_x - start_x + 1) * (end_y - start_y + 1) * (end_z - start_z + 1);
    ret.reserve(expected_size);
    for (int x = start_x; x <= end_x; x++) {
        for (int y = start_y; y <= end_y; y++) {
            for (int z = start_z; z <= end_z; z++) {
                std::vector<std::string> pieces;
                pieces.push_back(safe_to_string(server->minecraft->level->getTile(x, y, z)));
                if (!api_compat_mode) {
                    pieces.push_back(safe_to_string(server->minecraft->level->getData(x, y, z)));
                }
                ret.push_back(api_join_outputs(pieces, arg_separator));
            }
        }
    }
    if (ret.size() != expected_size) {
        IMPOSSIBLE();
    }
    // Return
    return api_join_outputs(ret, api_compat_mode ? arg_separator : list_separator);
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

// Handle Commands
std::string api_handle_world_command(const std::function<std::string()> &super, CommandServer *server, std::string_view &cmd, std::istringstream &args) {
    // Vanilla Commands
    passthrough(setBlock);
    passthrough(getBlock);
    passthrough(getBlockWithData);
    passthrough(setBlocks);
    passthrough(getHeight);
    passthrough(getPlayerIds);
    package(checkpoint) {
        passthrough(save);
        passthrough(restore);
    }
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
            if (player->username == username) {
                // Found
                return safe_to_string(player->id) + '\n';
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
            if (api_is_entity_selected(entity, type)) {
                result.push_back(api_get_entity_message(server, entity));
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
        return safe_to_string(result) + '\n';
    }
    command(removeEntities) {
        // Parse
        next_int(type);
        api_convert_to_mcpi_entity_type(type);
        // Remove
        int removed = 0;
        for (Entity *entity : server->minecraft->level->entities) {
            if (api_is_entity_selected(entity, type)) {
                entity->remove();
                removed++;
            }
        }
        return safe_to_string(removed) + '\n';
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
        return safe_to_string(entity->id) + '\n';
    }
    command(spawnItem) {
        // Parse
        next_float(x);
        next_float(y);
        next_float(z);
        next_int(item_id);
        next_int(count);
        next_int(data);
        // Translate
        server->pos_translator.from_float(x, y, z);
        // Check Item Type
        if (count <= 0 || !Item::items[item_id]) {
            return CommandServer::Fail;
        }
        ItemInstance item = {
            .count = count,
            .id = item_id,
            .auxiliary = data
        };
        // Spawn
        ItemEntity *entity = (ItemEntity *) misc_make_entity_from_id(
            server->minecraft->level,
            static_cast<int>(EntityType::DROPPED_ITEM),
            x, y, z,
            false
        );
        if (!entity) {
            return CommandServer::Fail;
        }
        entity->item = item;
        server->minecraft->level->addEntity((Entity *) entity);
        return safe_to_string(entity->id) + '\n';
    }

    // Get All Valid Entity Types
    command(getEntityTypes) {
        std::vector<std::string> result;
        for (const std::pair<const EntityType, std::pair<std::string, std::string>> &i : misc_get_entity_type_names()) {
            int id = static_cast<int>(i.first);
            api_convert_to_outside_entity_type(id);
            result.push_back(api_join_outputs({safe_to_string(id), api_get_output(i.second.second, true)}, arg_separator));
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
            for (std::string &line : sign->lines) {
                next_string(new_line, false);
                if (!api_compat_mode || !new_line_missing) {
                    line = api_get_input(new_line); \
                }
            }
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
        return safe_to_string(server->minecraft->level->data.seed) + '\n';
    }
    command(getGameMode) {
        return safe_to_string(server->minecraft->level->data.game_type) + '\n';
    }

    // Level Time
    command(setTime) {
        next_int(time);
        if (time < 0) {
            return CommandServer::Fail;
        }
        server->minecraft->level->data.time = time;
        return CommandServer::NullString;
    }
    command(getTime) {
        return safe_to_string(server->minecraft->level->data.time) + '\n';
    }

    // Invalid Command
    return CommandServer::Fail;
}