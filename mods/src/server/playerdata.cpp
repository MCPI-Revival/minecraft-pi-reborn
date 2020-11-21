#include <libcore/libcore.h>

#include "playerdata.h"
#include "server_internal.h"

#include "../minecraft.h"

// Execute Command Without LD_PRELOAD
static void exec_without_preload(std::string str) {
    std::string preload = getenv("LD_PRELOAD");
    unsetenv("LD_PRELOAD");
    system(str.c_str());
    setenv("LD_PRELOAD", preload.c_str(), 1);
}

// Get Player Data NBT Path
static std::string get_player_data_path(std::string username) {
    std::string path = std::string(getenv("HOME")) + "/.minecraft/games/com.mojang/minecraftWorlds/" + server_internal_get_world_name() + "/playerdata";
    exec_without_preload("mkdir -p " + path);
    return path + '/' + username + ".dat";
}

// Destruct Tag
static void destruct_tag(unsigned char *tag) {
    unsigned char *tag_vtable = *(unsigned char **) tag;
    Tag_deleteChildren_t Tag_deleteChildren = *(Tag_deleteChildren_t *) (tag_vtable + 0x8);
    (*Tag_deleteChildren)(tag);
    Tag_destructor_t Tag_destructor = *(Tag_destructor_t *) (tag_vtable + 0x4);
    (*Tag_destructor)(tag);
}

// Save Player Callback
static void save_player_callback(std::string username, unsigned char *player) {
    std::string nbt_file = get_player_data_path(username);

    // Open File
    FILE *file = fopen(nbt_file.c_str(), "wb");

    // Write Storage Version
    uint32_t storage_version = 3;
    fwrite(&storage_version, 4, 1, file);

    // Create Tag
    unsigned char *tag = (unsigned char *) ::operator new(0x38);
    (*CompoundTag)(tag);

    // Allocate
    RakNet_BitStream stream;
    RakDataOutput output;

    // Set VTable
    *(unsigned char **) &output.data[0] = RakDataOutput_vtable;

    // Construct BitStream
    (*RakNet_BitStream_constructor)(&stream);

    // Set BitStream
    *(RakNet_BitStream **) &output.data[4] = &stream;

    // Save NBT
    unsigned char *player_vtable = *(unsigned char **) player;
    Entity_saveWithoutId_t Entity_saveWithoutId = *(Entity_saveWithoutId_t *) (player_vtable + 0xcc);
    (*Entity_saveWithoutId)(player, tag);

    // Write NBT
    (*Tag_writeNamedTag)(tag, &output);

    // Destruct Tag
    destruct_tag(tag);

    // Write To File
    uint32_t numberOfBitsUsed = *(uint32_t *) &stream.data[0];
    uint32_t size = (numberOfBitsUsed + 7) >> 3;
    fwrite(&size, 4, 1, file);
    unsigned char *data = *(unsigned char **) &stream.data[12];
    fwrite(data, 1, size, file);

    // Destruct BitStream
    (*RakNet_BitStream_destructor)(&stream);

    // Close File
    fclose(file);
}

// Clear Player Inventory And Armor
static void clear_inventory(unsigned char *player) {
    unsigned char *inventory = *(unsigned char **) (player + 0xbe0);
    (*Inventory_clearInventoryWithDefault)(inventory);
    for (int i = 0; i < 4; i++) {
        (*Player_setArmor)(player, i, NULL);
    }
}

// Load Player Callback
static void load_player_callback(std::string username, unsigned char *player) {
    std::string nbt_file = get_player_data_path(username);

    // Open File
    FILE *file = fopen(nbt_file.c_str(), "rb");

    if (file) {
        uint32_t storage_version;
        size_t data_read = fread(&storage_version, 4, 1, file);

        // Check File
        if (data_read == 1 && storage_version == 3) {
            // Read Expected File Size
            uint32_t expected_size;
            data_read = fread(&expected_size, 4, 1, file);

            // Get Actual Size
            long int remaining = (*getRemainingFileSize)(file);

            if (data_read == 1 && expected_size > 0 && ((uint32_t) remaining) == expected_size) {
                // Read File
                unsigned char *data = (unsigned char *) malloc(expected_size);
                data_read = fread(data, 1, expected_size, file);

                if (data_read == expected_size) {
                    // Allocate
                    RakNet_BitStream stream;
                    RakDataInput input;

                    // Set VTable
                    *(unsigned char **) &input.data[0] = RakDataInput_vtable;

                    // Construct BitStream
                    (*RakNet_BitStream_constructor_with_data)(&stream, data, expected_size, false);

                    // Set BitStream
                    *(RakNet_BitStream **) &input.data[4] = &stream;

                    // Create Tag
                    unsigned char *tag = (*NbtIo_read)(&input);

                    if (tag != NULL) {
                        // Load Data
                        unsigned char *player_vtable = *(unsigned char **) player;
                        Entity_load_t Entity_load = *(Entity_load_t *) (player_vtable + 0xd0);
                        (*Entity_load)(player, tag);

                        // Clear Inventory Because The Client Will Also Have An Empty Inventory
                        clear_inventory(player);

                        // Destruct tag
                        destruct_tag(tag);
                    }

                    // Destruct BitStream
                    (*RakNet_BitStream_destructor)(&stream);
                }

                // Free Data
                free(data);
            }
        }

        // Close File
        fclose(file);
    }
}

static uint32_t get_entity_id(unsigned char *entity) {
    return *(uint32_t *) (entity + 0x1c);
}
static void ServerPlayer_moveTo_injection(unsigned char *player, float param_1, float param_2, float param_3, float param_4, float param_5) {
    // Call Original Method
    (*Entity_moveTo)(player, param_1, param_2, param_3, param_4, param_5);

    // Check If Player Is Spawned
    unsigned char *minecraft = *(unsigned char **) (player + 0xc8c);
    unsigned char *level = server_internal_get_level(minecraft);
    std::vector<unsigned char *> players = server_internal_get_players(level);
    bool spawned = false;
    uint32_t player_id = get_entity_id(player);
    for (std::size_t i = 0; i < players.size(); i++) {
        if (player_id == get_entity_id(players[i])) {
            spawned = true;
            break;
        }
    }
    
    // Load Data
    if (!spawned) {
        load_player_callback(server_internal_get_player_username(player), player);
    }
}

void playerdata_save(unsigned char *level) {
    // Save Players
    std::vector<unsigned char *> players = server_internal_get_players(level);
    for (std::size_t i = 0; i < players.size(); i++) {
        // Iterate Players
        unsigned char *player = players[i];
        std::string username = server_internal_get_player_username(player);
        save_player_callback(username, player);
    }
}

static void ServerSideNetworkHandler_onDisconnect_injection(unsigned char *server_side_network_handler, unsigned char *guid) {
    // Save Player Data
    unsigned char *player = (*ServerSideNetworkHandler_getPlayer)(server_side_network_handler, guid);
    if (player != NULL) {
        std::string username = server_internal_get_player_username(player);
        save_player_callback(username, player);
    }

    // Call Original Method
    (*ServerSideNetworkHandler_onDisconnect)(server_side_network_handler, guid);
}

void playerdata_init() {
    // Load Player NBT
    patch_address(ServerPlayer_moveTo_vtable_addr, (void *) ServerPlayer_moveTo_injection);
    // Save On Logout
    patch_address(ServerSideNetworkHandler_onDisconnect_vtable_addr, (void *) ServerSideNetworkHandler_onDisconnect_injection);
}