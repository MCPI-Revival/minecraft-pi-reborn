#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cmath>
#include <string>
#include <fstream>
#include <streambuf>

#include <GLES/gl.h>
#include <SDL/SDL.h>
#include <media-layer/core.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/input/input.h>
#include <mods/misc/misc.h>

#include "misc-internal.h"

// Sanitize Username
#define MAX_USERNAME_LENGTH 16
static void LoginPacket_read_injection(LoginPacket_read_t original, LoginPacket *packet, RakNet_BitStream *bit_stream) {
    // Call Original Method
    original(packet, bit_stream);

    // Prepare
    RakNet_RakString *rak_string = &packet->username;
    // Get Original Username
    const RakNet_RakString_SharedString *shared_string = rak_string->sharedString;
    const char *c_str = shared_string->c_str;
    // Sanitize
    std::string new_username = c_str;
    sanitize_string(new_username, MAX_USERNAME_LENGTH, 0);
    // Set New Username
    rak_string->Assign(new_username.c_str());
}

// Fix RakNet::RakString Security Bug
//
// RakNet::RakString's format constructor is often given unsanitized user input and is never used for formatting,
// this is a massive security risk, allowing clients to run arbitrary format specifiers, this disables the
// formatting functionality.
RakNet_RakString_constructor_2_t RakNet_RakString_constructor_2 = (RakNet_RakString_constructor_2_t) 0xea5cc;
static RakNet_RakString *RakNet_RakString_injection(RakNet_RakString *rak_string, const char *format, ...) {
    // Call Original Method
    return RakNet_RakString_constructor_2(rak_string, "%s", format);
}

// Print Error Message If RakNet Startup Fails
static const char *RAKNET_ERROR_NAMES[] = {
    "Success",
    "Already Started",
    "Invalid Socket Descriptors",
    "Invalid Max Connections",
    "Socket Family Not Supported",
    "Part Already In Use",
    "Failed To Bind Port",
    "Failed Test Send",
    "Port Cannot Be 0",
    "Failed To Create Network Thread",
    "Unknown"
};
static RakNet_StartupResult RakNetInstance_host_RakNet_RakPeer_Startup_injection(RakNet_RakPeer *rak_peer, unsigned short maxConnections, unsigned char *socketDescriptors, uint32_t socketDescriptorCount, int32_t threadPriority) {
    // Call Original Method
    const RakNet_StartupResult result = rak_peer->Startup(maxConnections, socketDescriptors, socketDescriptorCount, threadPriority);

    // Print Error
    if (result != RAKNET_STARTED) {
        CONDITIONAL_ERR(reborn_is_server(), "Failed To Start RakNet: %s", RAKNET_ERROR_NAMES[result]);
    }

    // Return
    return result;
}

// Fix Furnace Not Checking Item Auxiliary When Inserting New Item
static int32_t FurnaceScreen_handleAddItem_injection(FurnaceScreen_handleAddItem_t original, FurnaceScreen *furnace_screen, int32_t slot, const ItemInstance *item) {
    // Get Existing Item
    FurnaceTileEntity *tile_entity = furnace_screen->tile_entity;
    const ItemInstance *existing_item = tile_entity->getItem(slot);

    // Check Item
    int valid;
    if (item->id == existing_item->id && item->auxiliary == existing_item->auxiliary) {
        // Item Matches, Is Valid
        valid = 1;
    } else {
        // Item Doesn't Match, Check If Existing Item Is Empty
        if ((existing_item->id | existing_item->count | existing_item->auxiliary) == 0) {
            // Existing Item Is Empty, Is Valid
            valid = 1;
        } else {
            // Existing Item Isn't Empty, Isn't Valid
            valid = 0;
        }
    }

    // Call Original Method
    if (valid) {
        // Valid
        return original(furnace_screen, slot, item);
    } else {
        // Invalid
        return 0;
    }
}

// Get Real Selected Slot
int32_t misc_get_real_selected_slot(const Player *player) {
    // Get Selected Slot
    const Inventory *inventory = player->inventory;
    int32_t selected_slot = inventory->selectedSlot;

    // Linked Slots
    const int32_t linked_slots_length = inventory->linked_slots_length;
    if (selected_slot < linked_slots_length) {
        const int32_t *linked_slots = inventory->linked_slots;
        selected_slot = linked_slots[selected_slot];
    }

    // Return
    return selected_slot;
}

// Custom API Port
HOOK(bind, int, (int sockfd, const struct sockaddr *addr, socklen_t addrlen)) {
    const sockaddr *new_addr = addr;
    sockaddr_in in_addr = {};
    if (addr->sa_family == AF_INET) {
        in_addr = *(const sockaddr_in *) new_addr;
        if (in_addr.sin_port == ntohs(4711)) {
            const char *new_port_str = getenv(MCPI_API_PORT_ENV);
            long int new_port;
            if (new_port_str != nullptr && (new_port = strtol(new_port_str, nullptr, 0)) != 0L) {
                in_addr.sin_port = htons(new_port);
            }
        }
        new_addr = (const sockaddr *) &in_addr;
    }
    return real_bind()(sockfd, new_addr, addrlen);
}

// Generate Caves
static void RandomLevelSource_buildSurface_injection(RandomLevelSource_buildSurface_t original, RandomLevelSource *random_level_source, int32_t chunk_x, int32_t chunk_y, unsigned char *chunk_data, Biome **biomes) {
    // Call Original Method
    original(random_level_source, chunk_x, chunk_y, chunk_data, biomes);

    // Get Level
    Level *level = random_level_source->level;

    // Get Cave Feature
    LargeCaveFeature *cave_feature = &random_level_source->cave_feature;

    // Generate
    cave_feature->apply((ChunkSource *) random_level_source, level, chunk_x, chunk_y, chunk_data, 0);
}

// Disable Hostile AI In Creative Mode
static Entity *PathfinderMob_findAttackTarget_injection(PathfinderMob *mob) {
    // Call Original Method
    Entity *target = mob->findAttackTarget();

    // Only modify the AI of monsters
    if (mob->getCreatureBaseType() != 1) {
        return target;
    }

    // Check If Creative Mode
    if (target != nullptr && target->isPlayer()) {
        const Player *player = (Player *) target;
        const Inventory *inventory = player->inventory;
        const bool is_creative = inventory->is_creative;
        if (is_creative) {
            target = nullptr;
        }
    }

    // Return
    return target;
}

// Fix used items transferring durability
static int selected_slot = -1;
static void Player_startUsingItem_injection(Player_startUsingItem_t original, Player *self, ItemInstance *item_instance, const int time) {
    selected_slot = self->inventory->selectedSlot;
    original(self, item_instance, time);
}
static void Player_stopUsingItem_injection(Player_stopUsingItem_t original, Player *self) {
    if (selected_slot != self->inventory->selectedSlot) {
        self->itemBeingUsed.id = 0;
    }
    original(self);
}

// Read Asset File
static AppPlatform_readAssetFile_return_value AppPlatform_readAssetFile_injection(__attribute__((unused)) AppPlatform_readAssetFile_t original, __attribute__((unused)) AppPlatform *app_platform, const std::string &path) {
    // Open File
    std::ifstream stream("data/" + path, std::ios_base::binary | std::ios_base::ate);
    if (!stream) {
        // Does Not Exist
        AppPlatform_readAssetFile_return_value ret;
        ret.length = -1;
        ret.data = nullptr;
        return ret;
    }
    // Read File
    std::streamoff len = stream.tellg();
    char *buf = new char[len];
    ALLOC_CHECK(buf);
    stream.seekg(0, std::ifstream::beg);
    stream.read(buf, len);
    stream.close();
    // Return String
    AppPlatform_readAssetFile_return_value ret;
    ret.length = int(len);
    ret.data = strdup(buf);
    return ret;
}

// Implement crafting remainders
static void PaneCraftingScreen_craftSelectedItem_PaneCraftingScreen_recheckRecipes_injection(PaneCraftingScreen *self) {
    // Check for crafting remainders
    const CItem *item = self->item;
    for (size_t i = 0; i < item->ingredients.size(); i++) {
        ItemInstance requested_item_instance = item->ingredients[i].requested_item;
        Item *requested_item = Item::items[requested_item_instance.id];
        ItemInstance *craftingRemainingItem = requested_item->getCraftingRemainingItem(&requested_item_instance);
        if (craftingRemainingItem != nullptr) {
            // Add or drop remainder
            LocalPlayer *player = self->minecraft->player;
            if (!player->inventory->add(craftingRemainingItem)) {
                // Drop
                player->drop(craftingRemainingItem, false);
            }
        }
    }
    // Call Original Method
    self->recheckRecipes();
}
static ItemInstance *Item_getCraftingRemainingItem_injection(__attribute__((unused)) Item_getCraftingRemainingItem_t original, const Item *self, const ItemInstance *item_instance) {
    if (self->craftingRemainingItem != nullptr) {
        ItemInstance *ret = new ItemInstance;
        ret->id = self->craftingRemainingItem->id;
        ret->count = item_instance->count;
        ret->auxiliary = 0;
        return ret;
    }
    return nullptr;
}

// Display Date In Select World Screen
static std::string AppPlatform_linux_getDateString_injection(__attribute__((unused)) AppPlatform_linux *app_platform, const int time) {
    // From https://github.com/ReMinecraftPE/mcpe/blob/56e51027b1c2e67fe5a0e8a091cefe51d4d11926/platforms/sdl/base/AppPlatform_sdl_base.cpp#L68-L84
    const time_t tt = time;
    tm t = {};
    gmtime_r(&tt, &t);
    char buf[2048];
    strftime(buf, sizeof buf, "%b %d %Y %H:%M:%S", &t);
    return std::string(buf);
}

// Missing Strings
static void add_missing_string(const std::string &key, const std::string &value) {
    if (!I18n::_strings.contains(key)) {
        I18n::_strings[key] = value;
    }
}
static void Language_injection() {
    // Fix Language Strings
    add_missing_string("tile.waterStill.name", "Still Water");
    add_missing_string("tile.lavaStill.name", "Still Lava");
    add_missing_string("tile.grassCarried.name", "Carried Grass");
    add_missing_string("tile.leavesCarried.name", "Carried Leaves");
    add_missing_string("tile.invBedrock.name", "Invisible Bedrock");
    // Missing Language Strings
    add_missing_string("item.camera.name", "Camera");
    add_missing_string("item.seedsMelon.name", "Melon Seeds");
    add_missing_string("tile.pumpkinStem.name", "Pumpkin Stem");
    add_missing_string("tile.stoneSlab.name", "Double Stone Slab");
}
// Invisible Bedrock
static Tile *Tile_initTiles_Tile_init_invBedrock_injection(Tile *t) {
    Tile *ret = t->init();
    t->setDescriptionId("invBedrock");
    return ret;
}
// Append "Still" Suffix To Liquid Description Keys
static std::string *Tile_initTiles_std_string_constructor(std::string *self, const char *from, const std::string::allocator_type &alloc) {
    new (self) std::string(from, alloc);
    self->append("Still");
    return self;
}

// Fix Pigmen Burning In The Sun
static bool fix_pigmen_burning = false;
static float Zombie_aiStep_getBrightness_injection(Entity *self, float param_1) {
    if (fix_pigmen_burning && self->getEntityTypeId() == 36) {
        return 0;
    } else {
        return self->getBrightness(param_1);
    }
}

// Fix Door Item Dropping
static void DoorTile_neighborChanged_Tile_spawnResources_injection(DoorTile *self, Level *level, int x, int y, int z, int data2, __attribute__((unused)) float chance) {
    self->spawnResources(level, x, y, z, data2, 1);
}

// Fix Cobweb Lighting
static Tile *Tile_initTiles_WebTile_setLightBlock_injection(Tile *self, __attribute__((unused)) int strength) {
    return self;
}

// Fix Fire Immunity
static void Mob_baseTick_injection_fire_immunity(Mob_baseTick_t original, Mob *self) {
    // Fix Fire Timer
    if (self->fire_immune) {
        self->fire_timer = 0;
    }
    // Call Original Method
    original(self);
}

// Fix Fire Syncing
#define FLAG_ONFIRE 0
static void Mob_baseTick_injection_fire_syncing(Mob_baseTick_t original, Mob *self) {
    // Fix Fire Timer
    if (self->level->is_client_side) {
        self->fire_timer = 0;
    }
    // Call Original Method
    original(self);
    // Sync Data
    if (!self->level->is_client_side) {
        self->setSharedFlag(FLAG_ONFIRE, self->fire_timer > 0);
    }
}
static bool Entity_isOnFire_injection(Entity_isOnFire_t original, Entity *self) {
    // Call Original Method
    bool ret = original(self);

    // Check Shared Data
    bool shared_data = false;
    if (self->isMob()) {
        shared_data = ((Mob *) self)->getSharedFlag(FLAG_ONFIRE);
    }
    if (shared_data) {
        ret = true;
    }

    // Return
    return ret;
}

// Fix Sneaking Syncing
#define FLAG_SNEAKING 1
#define PLAYER_ACTION_STOP_SNEAKING 100
#define PLAYER_ACTION_START_SNEAKING 101
static void LocalPlayer_tick_injection(LocalPlayer_tick_t original, LocalPlayer *self) {
    // Call Original Method
    original(self);
    // Sync Data
    if (!self->level->is_client_side) {
        self->setSharedFlag(FLAG_SNEAKING, self->isSneaking());
    } else {
        const bool real = self->isSneaking();
        const bool synced = self->getSharedFlag(FLAG_SNEAKING);
        if (real != synced) {
            // Send To Server
            PlayerActionPacket *packet = PlayerActionPacket::allocate();
            Packet_constructor->get(false)((Packet *) packet);
            packet->vtable = PlayerActionPacket_vtable::base;
            packet->entity_id = self->id;
            packet->action = real ? PLAYER_ACTION_START_SNEAKING : PLAYER_ACTION_STOP_SNEAKING;
            self->minecraft->rak_net_instance->send(*(Packet *) packet);
        }
    }
}
static void ServerSideNetworkHandler_handle_PlayerActionPacket_injection(ServerSideNetworkHandler_handle_PlayerActionPacket_t original, ServerSideNetworkHandler *self, const RakNet_RakNetGUID &rak_net_guid, PlayerActionPacket *packet) {
    // Call Original Method
    original(self, rak_net_guid, packet);

    // Handle Sneaking
    const bool is_sneaking = packet->action == PLAYER_ACTION_START_SNEAKING;
    if (self->level != nullptr && (is_sneaking || packet->action == PLAYER_ACTION_STOP_SNEAKING)) {
        Entity *entity = self->level->getEntity(packet->entity_id);
        if (entity != nullptr && entity->isPlayer()) {
            ((Player *) entity)->setSharedFlag(FLAG_SNEAKING, is_sneaking);
        }
    }
}

// Make Mobs Actually Catch On Fire
static void set_on_fire(Mob *mob, const int seconds) {
    const int value = seconds * 20;
    if (value > mob->fire_timer) {
        mob->fire_timer = value;
    }
}
template <typename Self>
static void Monster_aiStep_injection(__attribute__((unused)) const std::function<void(Self *)> &original, Self *self) {
    // Fire!
    Level *level = self->level;
    if (level->isDay() && !level->is_client_side) {
        const float brightness = Zombie_aiStep_getBrightness_injection((Entity *) self, 1);
        if (brightness > 0.5f) {
            Random *random = &self->random;
            if (level->canSeeSky(Mth::floor(self->x), Mth::floor(self->y), Mth::floor(self->z)) && random->nextFloat() * 3.5f < (brightness - 0.4f)) {
                set_on_fire((Mob *) self, 8);
            }
        }
    }

    // Call Parent Method
    Monster_aiStep->get(false)((Monster *) self);
}

// Clear Fire For Creative Players
static void Player_tick_injection(Player_tick_t original, Player *self) {
    // Fix Value
    if (self->inventory->is_creative && !self->level->is_client_side && self->isOnFire()) {
        self->fire_timer = 0;
    }
    // Call Original Method
    original(self);
}

// Rare Segfault
static int Dimension_isValidSpawn_Level_getTopTile_injection(Level *self, int x, int z) {
    int ret = self->getTopTile(x, z);
    if (ret == 0) {
        ret = Tile::invisible_bedrock->id;
    }
    return ret;
}

// Prevent Sugar From Being "handEquipped()"
static Item *Item_initItems_Item_handEquipped_injection(Item *self) {
    return self;
}

// Init
void init_misc() {
    // Sanitize Username
    if (feature_has("Sanitize Usernames", server_enabled)) {
        overwrite_calls(LoginPacket_read, LoginPacket_read_injection);
    }

    // Fix RakNet::RakString Security Bug
    if (feature_has("Patch RakNet Security Bug", server_enabled)) {
        overwrite_calls_manual((void *) RakNet_RakString_constructor_2, (void *) RakNet_RakString_injection);
    }

    // Print Error Message If RakNet Startup Fails
    if (feature_has("Log RakNet Startup Errors", server_enabled)) {
        overwrite_call((void *) 0x73778, (void *) RakNetInstance_host_RakNet_RakPeer_Startup_injection);
    }

    // Fix Furnace Not Checking Item Auxiliary When Inserting New Item
    if (feature_has("Fix Furnace Not Checking Item Auxiliary", server_disabled)) {
        overwrite_calls(FurnaceScreen_handleAddItem, FurnaceScreen_handleAddItem_injection);
    }

    // Disable Speed Bridging
    if (feature_has("Disable Speed Bridging", server_disabled)) {
        unsigned char disable_speed_bridging_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r3, r3"
        patch((void *) 0x494b4, disable_speed_bridging_patch);
    }

    // Disable Creative Mode Mining Delay
    if (feature_has("Disable Creative Mode Mining Delay", server_disabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x19fa0, nop_patch);
    }

    // Generate Caves
    if (feature_has("Generate Caves", server_auto)) {
        overwrite_calls(RandomLevelSource_buildSurface, RandomLevelSource_buildSurface_injection);
    }

    // Disable Hostile AI In Creative Mode
    if (feature_has("Disable Hostile AI In Creative Mode", server_enabled)) {
        overwrite_call((void *) 0x83b8c, (void *) PathfinderMob_findAttackTarget_injection);
    }

    // Send The Full Level, Not Only Changed Chunks
    if (feature_has("Send Full Level When Hosting Game", server_enabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x717c4, nop_patch);
        unsigned char mov_r3_ff[4] = {0xff, 0x30, 0xa0, 0xe3}; // "mov r3, #0xff"
        patch((void *) 0x7178c, mov_r3_ff);
    }

    // Fix Used Items Transferring Durability
    if (feature_has("Fix Transferring Durability When Using Items", server_disabled)) {
        overwrite_calls(Player_startUsingItem, Player_startUsingItem_injection);
        overwrite_calls(Player_stopUsingItem, Player_stopUsingItem_injection);
    }

    // Implement AppPlatform::readAssetFile So Translations Work
    if (feature_has("Load Language Files", server_enabled)) {
        overwrite_calls(AppPlatform_readAssetFile, AppPlatform_readAssetFile_injection);
    }

    // Implement Crafting Remainders
    if (feature_has("Implement Crafting Remainders", server_enabled)) {
        overwrite_call((void *) 0x2e230, (void *) PaneCraftingScreen_craftSelectedItem_PaneCraftingScreen_recheckRecipes_injection);
        overwrite_calls(Item_getCraftingRemainingItem, Item_getCraftingRemainingItem_injection);
    }

    // Display Date In Select World Screen
    if (feature_has("Display Date In Select World Screen", server_disabled)) {
        patch_vtable(AppPlatform_linux_getDateString, AppPlatform_linux_getDateString_injection);
    }

    // Fullscreen
    if (feature_has("Fullscreen Support", server_disabled)) {
        misc_run_on_key_press([](__attribute__((unused)) Minecraft *mc, int key) {
            if (key == MC_KEY_F11) {
                media_toggle_fullscreen();
                return true;
            } else {
                return false;
            }
        });
    }

    // Fix/Update Language Strings
    if (feature_has("Add Missing Language Strings", server_disabled)) {
        misc_run_on_language_setup(Language_injection);
        // Water/Lava Language Strings
        overwrite_call((void *) 0xc3b54, (void *) Tile_initTiles_std_string_constructor);
        overwrite_call((void *) 0xc3c7c, (void *) Tile_initTiles_std_string_constructor);
        // Carried Tile Language Strings
        patch_address((void *) 0xc6674, (void *) "grassCarried");
        patch_address((void *) 0xc6684, (void *) "leavesCarried");
        // Invisible Bedrock Language String
        overwrite_call((void *) 0xc5f04, (void *) Tile_initTiles_Tile_init_invBedrock_injection);
    }

    // Prevent Pigmen From Burning In The Sun
    if (feature_has("Fix Pigmen Burning In The Sun", server_enabled)) {
        fix_pigmen_burning = true;
        overwrite_call((void *) 0x89a1c, (void *) Zombie_aiStep_getBrightness_injection);
    }

    // Fix Door Duplication
    if (feature_has("Fix Door Duplication", server_enabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0xbe230, nop_patch);
        overwrite_call((void *) 0xbe110, (void *) DoorTile_neighborChanged_Tile_spawnResources_injection);
    }

    // Fix Cobweb Lighting
    if (feature_has("Fix Cobweb Lighting", server_enabled)) {
        overwrite_call((void *) 0xc444c, (void *) Tile_initTiles_WebTile_setLightBlock_injection);
    }

    // Fix Fire Immunity
    if (feature_has("Fix Fire Immunity", server_enabled)) {
        overwrite_calls(Mob_baseTick, Mob_baseTick_injection_fire_immunity);
    }

    // Fix Fire Syncing
    if (feature_has("Fix Fire Syncing", server_enabled)) {
        overwrite_calls(Mob_baseTick, Mob_baseTick_injection_fire_syncing);
        overwrite_calls(Entity_isOnFire, Entity_isOnFire_injection);
    }

    // Fix Sneaking Syncing
    if (feature_has("Fix Sneaking Syncing", server_enabled)) {
        overwrite_calls(LocalPlayer_tick, LocalPlayer_tick_injection);
        overwrite_calls(ServerSideNetworkHandler_handle_PlayerActionPacket, ServerSideNetworkHandler_handle_PlayerActionPacket_injection);
    }

    // Make Skeletons/Zombies Actually Catch On Fire
    if (feature_has("Fix Sunlight Not Properly Setting Mobs On Fire", server_enabled)) {
        overwrite_calls(Zombie_aiStep, Monster_aiStep_injection<Zombie>);
        overwrite_calls(Skeleton_aiStep, Monster_aiStep_injection<Skeleton>);
    }

    // Clear Fire For Creative Players
    if (feature_has("Stop Creative Players From Burning", server_enabled)) {
        overwrite_calls(Player_tick, Player_tick_injection);
    }

    // Rare Segfault
    if (feature_has("Fix Crash When Generating Certain Seeds", server_enabled)) {
        overwrite_call((void *) 0xb198c, (void *) Dimension_isValidSpawn_Level_getTopTile_injection);
    }

    // Fix Sugar Rendering
    if (feature_has("Fix Sugar Position In Hand", server_disabled)) {
        overwrite_call((void *) 0x976f8, (void *) Item_initItems_Item_handEquipped_injection);
    }

    // Disable overwrite_calls() After Minecraft::init
    misc_run_on_init([](__attribute__((unused)) Minecraft *minecraft) {
        thunk_enabler = [](__attribute__((unused)) void *a, __attribute__((unused)) void *b) -> void * {
            IMPOSSIBLE();
        };
    });

    // Init Other Components
    _init_misc_tinting();
    _init_misc_ui();
    _init_misc_logging();
    _init_misc_api();
    _init_misc_graphics();
}
