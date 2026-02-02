#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <string>
#include <fstream>
#include <streambuf>

#include <media-layer/core.h>

#include <libreborn/patch.h>
#include <libreborn/util/string.h>
#include <libreborn/util/util.h>

#include <symbols/LoginPacket.h>
#include <symbols/RakNet_RakString_SharedString.h>
#include <symbols/FurnaceScreen.h>
#include <symbols/FurnaceTileEntity.h>
#include <symbols/ItemInstance.h>
#include <symbols/FillingContainer.h>
#include <symbols/Player.h>
#include <symbols/Inventory.h>
#include <symbols/RandomLevelSource.h>
#include <symbols/PathfinderMob.h>
#include <symbols/Entity.h>
#include <symbols/AppPlatform.h>
#include <symbols/PaneCraftingScreen.h>
#include <symbols/CItem.h>
#include <symbols/Item.h>
#include <symbols/Minecraft.h>
#include <symbols/LocalPlayer.h>
#include <symbols/I18n.h>
#include <symbols/Tile.h>
#include <symbols/DoorTile.h>
#include <symbols/Level.h>
#include <symbols/Mth.h>
#include <symbols/Monster.h>
#include <symbols/ChestTile.h>
#include <symbols/Screen.h>
#include <symbols/TextEditScreen.h>
#include <symbols/SignTileEntity.h>
#include <symbols/TorchTile.h>
#include <symbols/Chicken.h>
#include <symbols/RegionFile.h>
#include <symbols/AppPlatform_linux.h>
#include <symbols/Zombie.h>
#include <symbols/Skeleton.h>
#include <symbols/NinecraftApp.h>
#include <symbols/BiomeSource.h>
#include <symbols/ReedsFeature.h>
#include <symbols/Feature.h>
#include <symbols/Biome.h>
#include <symbols/CreatorMode.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/input/input.h>
#include <mods/multiplayer/packets.h>
#include <mods/misc/misc.h>

#include "internal.h"

// Sanitize Username
#define MAX_USERNAME_LENGTH 16
void misc_sanitize_username(std::string &username) {
    sanitize_string(username, MAX_USERNAME_LENGTH, false);
}
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
    misc_sanitize_username(new_username);
    // Set New Username
    rak_string->Assign(new_username.c_str());
}

// Fix Furnace Not Checking Item Auxiliary When Inserting New Item
static bool FurnaceScreen_handleAddItem_injection(FurnaceScreen_handleAddItem_t original, FurnaceScreen *furnace_screen, int32_t slot, const ItemInstance *item) {
    // Get Existing Item
    FurnaceTileEntity *tile_entity = furnace_screen->tile_entity;
    const ItemInstance *existing_item = tile_entity->getItem(slot);

    // Check Item
    bool valid;
    if (item->id == existing_item->id && item->auxiliary == existing_item->auxiliary) {
        // Item Matches, Is Valid
        valid = true;
    } else {
        // Item Doesn't Match, Check If Existing Item Is Empty
        if ((existing_item->id | existing_item->count | existing_item->auxiliary) == 0) {
            // Existing Item Is Empty, Is Valid
            valid = true;
        } else {
            // Existing Item Isn't Empty, Isn't Valid
            valid = false;
        }
    }

    // Call Original Method
    if (valid) {
        // Valid
        return original(furnace_screen, slot, item);
    } else {
        // Invalid
        return false;
    }
}
static bool FurnaceScreen_moveOver_FillingContainer_removeResource_two_injection(FillingContainer *self, const ItemInstance &item) {
    return self->removeResource_one(item, true) == 0;
}

// Fix Creative Mode Mining Delay
static void CreatorMode_stopDestroyBlock_injection(CreatorMode_stopDestroyBlock_t original, CreatorMode *self) {
    // Call Original Method
    original(self);
    // Reset Delay
    self->destroy_delay = 0;
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

// Generate Caves
static void RandomLevelSource_buildSurface_injection(RandomLevelSource_buildSurface_t original, RandomLevelSource *random_level_source, int32_t chunk_x, int32_t chunk_y, unsigned char *chunk_data, Biome **biomes) {
    // Call Original Method
    original(random_level_source, chunk_x, chunk_y, chunk_data, biomes);

    // Generate
    Level *level = random_level_source->level;
    LargeCaveFeature *cave_feature = &random_level_source->cave_feature;
    cave_feature->apply((ChunkSource *) random_level_source, level, chunk_x, chunk_y, chunk_data, 0);
}

// Generate Tall Grass
static Biome *tall_grass_biome = nullptr;
static Biome *RandomLevelSource_postProcess_BiomeSource_getBiome_injection(BiomeSource *self, const int x, const int z) {
    tall_grass_biome = self->getBiome(x, z);
    return tall_grass_biome;
}
static bool RandomLevelSource_postProcess_ReedsFeature_place_injection(ReedsFeature *self, Level *level, Random *random, int x, int y, int z) {
    // Place Tall Grass
    if (tall_grass_biome) {
        // Repurpose First Loop Iteration
        bool ret = false;
        Feature *feature = tall_grass_biome->getGrassFeature(random);
        if (feature) {
            ret = feature->place(level, random, x, y, z);
            feature->destructor_deleting();
        }
        tall_grass_biome = nullptr;
        return ret;
    }

    // Call Original Method
    return self->place(level, random, x, y, z);
}

// Add Tall Gras To Bone-Meal
static int DyePowderItem_useOn_Level_getTile_injection(Level *self, const int x, const int y, const int z) {
    // Call Original Method
    const int tile = self->getTile(x, y, z);

    // Try Spawning Tall Grass
    if (tile == 0) {
        const int rng = Item::random.nextInt(10);
        if (rng != 0) {
            const int new_tile = Tile::tallgrass->id;
            self->setTileAndData(x, y, z, new_tile, 1);
            return new_tile;
        }
    }

    // Return
    return tile;
}

// Disable Hostile AI In Creative Mode
static Entity *PathfinderMob_updateAi_PathfinderMob_findAttackTarget_injection(PathfinderMob *mob) {
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
static AppPlatform_readAssetFile_return_value AppPlatform_readAssetFile_injection(MCPI_UNUSED AppPlatform_readAssetFile_t original, MCPI_UNUSED AppPlatform *app_platform, const std::string &path) {
    // Open File
    std::ifstream stream("data/" + path, std::ios::binary | std::ios::ate);
    if (!stream) {
        // Does Not Exist
        AppPlatform_readAssetFile_return_value ret = {};
        ret.length = -1;
        ret.data = nullptr;
        return ret;
    }
    // Read File
    const std::streamoff len = stream.tellg();
    char *buf = new char[len];
    stream.seekg(0, std::ifstream::beg);
    stream.read(buf, len);
    stream.close();
    // Return String
    AppPlatform_readAssetFile_return_value ret = {};
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
            // Add or drop the remainder.
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
static ItemInstance *Item_getCraftingRemainingItem_injection(MCPI_UNUSED Item_getCraftingRemainingItem_t original, const Item *self, const ItemInstance *item_instance) {
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
static std::string AppPlatform_linux_getDateString_injection(MCPI_UNUSED AppPlatform_linux *app_platform, const int time) {
    // From https://github.com/ReMinecraftPE/mcpe/blob/56e51027b1c2e67fe5a0e8a091cefe51d4d11926/platforms/sdl/base/AppPlatform_sdl_base.cpp#L68-L84
    return format_time("%b %d %Y %H:%M:%S", time);
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
static void DoorTile_neighborChanged_Tile_spawnResources_injection(DoorTile *self, Level *level, int x, int y, int z, int data2, MCPI_UNUSED float chance) {
    self->spawnResources(level, x, y, z, data2, 1);
}

// Fix Cobweb Lighting
static Tile *Tile_initTiles_WebTile_setLightBlock_injection(Tile *self, MCPI_UNUSED int strength) {
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

// Make Mobs Actually Catch On Fire
void misc_set_on_fire(Mob *mob, const int seconds) {
    const int value = seconds * 20;
    if (value > mob->fire_timer) {
        mob->fire_timer = value;
    }
}
template <typename Self>
static void Monster_aiStep_injection(MCPI_UNUSED const std::function<void(Self *)> &original, Self *self) {
    // Fire!
    Level *level = self->level;
    if (level->isDay() && !level->is_client_side) {
        const float brightness = Zombie_aiStep_getBrightness_injection((Entity *) self, 1);
        if (brightness > 0.5f) {
            Random *random = &self->random;
            if (level->canSeeSky(Mth::floor(self->x), Mth::floor(self->y), Mth::floor(self->z)) && random->nextFloat() * 3.5f < (brightness - 0.4f)) {
                misc_set_on_fire((Mob *) self, 8);
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

// Chest Placement
static bool ChestTile_mayPlace_injection(MCPI_UNUSED ChestTile_mayPlace_t original, MCPI_UNUSED ChestTile *self, MCPI_UNUSED Level *level, MCPI_UNUSED int x, MCPI_UNUSED int y, MCPI_UNUSED int z, MCPI_UNUSED uchar face) {
    return true;
}

// Fix Sign Crash
static TextEditScreen *current_sign_screen = nullptr;
static void Minecraft_setScreen_injection(Minecraft_setScreen_t original, Minecraft *self, Screen *screen) {
    // Call Original Method
    original(self, screen);
    // Track
    current_sign_screen = nullptr;
    if (screen != nullptr && screen->vtable == (Screen::VTable *) TextEditScreen::VTable::base) {
        current_sign_screen = (TextEditScreen *) screen;
    }
}
static SignTileEntity *SignTileEntity_destructor_injection(SignTileEntity_destructor_complete_t original, SignTileEntity *self) {
    // Close Screen
    if (current_sign_screen != nullptr && current_sign_screen->sign == self) {
        current_sign_screen->sign = nullptr;
        current_sign_screen->minecraft->setScreen(nullptr);
    }
    // Call Original Method
    return original(self);
}

// Fix Spawn Point Hanging
static constexpr int allowed_spawn_point_tries = 10000;
static constexpr int not_finding_spawn_point = -1;
static int spawn_point_tries = not_finding_spawn_point;
static void Level_setInitialSpawn_or_validateSpawn_injection(Level_setInitialSpawn_t original, Level *self) {
    spawn_point_tries = allowed_spawn_point_tries;
    original(self);
    spawn_point_tries = not_finding_spawn_point;
}
static int Level_getTopTile_injection(Level_getTopTile_t original, Level *self, int x, int z) {
    bool actually_check = true;
    if (spawn_point_tries == 0) {
        // Give Up Searching
        actually_check = false;
    } else if (spawn_point_tries > 0) {
        // Count Down
        spawn_point_tries--;
    }
    if (actually_check) {
        return original(self, x, z);
    } else {
        return Tile::bedrock->id;
    }
}

// Fix Torch Placement
static void TorchTile_onPlace_injection(MCPI_UNUSED TorchTile_onPlace_t original, TorchTile *self, Level *level, const int x, const int y, const int z) {
    Tile_onPlace->get(false)((Tile *) self, level, x, y, z);
}
static int TorchTile_getPlacedOnFaceDataValue_injection(MCPI_UNUSED TorchTile_getPlacedOnFaceDataValue_t original, MCPI_UNUSED TorchTile *self, Level *level, const int x, const int y, const int z, const int face, MCPI_UNUSED const float hit_x, MCPI_UNUSED const float hit_y, MCPI_UNUSED const float hit_z, const int item_auxiliary) {
    int data = item_auxiliary;
#define test(a, b, ...) if ((data == 0 || face == (a)) && __VA_ARGS__) data = (b)
    test(1, 5, TorchTile::isConnection(level, x, y - 1, z));
    test(2, 4, level->isSolidBlockingTile(x, y, z + 1));
    test(3, 3, level->isSolidBlockingTile(x, y, z - 1));
    test(4, 2, level->isSolidBlockingTile(x + 1, y, z));
    test(5, 1, level->isSolidBlockingTile(x - 1, y, z));
#undef test
    return data;
}

// Fix Egg Behavior
static void ThrownEgg_onHit_Chicken_moveTo_injection(Chicken *self, const float x, const float y, const float z, const float yaw, const float pitch) {
    // Call Original Method
    self->moveTo(x, y, z, yaw, pitch);
    // Fix Health
    self->health = self->getMaxHealth();
    // Fix Age
    self->setAge(-24000);
}

// Fix Dropping Armor On Death
static void LocalPlayer_die_injection(LocalPlayer_die_t original, LocalPlayer *self, Entity *cause) {
    // Call Original Method
    original(self, cause);
    // Properly Drop Armor
    for (int i = 0; i < multiplayer_armor_size; i++) {
        const ItemInstance *item = self->getArmor(i);
        if (ItemInstance::isArmorItem(item)) {
            ItemInstance *new_item = new ItemInstance;
            *new_item = *item;
            self->drop(new_item, true);
            self->setArmor(i, nullptr);
        }
    }
}

// Reliable Chunk Saving
static bool RegionFile_writeChunk_injection(RegionFile_writeChunk_t original, RegionFile *self, const int x, const int z, RakNet_BitStream &data) {
    // Call Original Method
    const bool ret = original(self, x, z, data);
    // Sync
    FILE *file = (FILE *) self->file;
    fflush(file);
    // Return
    return ret;
}

// Force Saving Level Data On World Exit
static void Minecraft_leaveGame_injection(Minecraft_leaveGame_t original, Minecraft *self, const bool save_remote) {
    // Save
    Level *level = self->level;
    if (level) {
        const bool is_generating = self->generating_level || !self->level_generation_signal;
        const bool should_save = !level->is_client_side || save_remote;
        if (!is_generating && should_save) {
            level->saveGame();
        }
    }
    // Call Original Method
    original(self, save_remote);
}

// Init
void init_misc() {
    // Sanitize Username
    if (feature_has("Sanitize Usernames", server_enabled)) {
        overwrite_calls(LoginPacket_read, LoginPacket_read_injection);
    }

    // Fix Furnace Not Checking Item Auxiliary When Inserting New Item
    if (feature_has("Fix Furnace Not Checking Item Auxiliary", server_disabled)) {
        overwrite_calls(FurnaceScreen_handleAddItem, FurnaceScreen_handleAddItem_injection);
        overwrite_call((void *) 0x32580, FillingContainer_removeResource_two, FurnaceScreen_moveOver_FillingContainer_removeResource_two_injection);
    }

    // Disable Speed Bridging
    if (feature_has("Disable Speed Bridging", server_disabled)) {
        unsigned char disable_speed_bridging_patch[4] = {0x03, 0x00, 0x53, 0xe1}; // "cmp r3, r3"
        patch((void *) 0x494b4, disable_speed_bridging_patch);
    }

    // Disable Creative Mode Mining Delay
    if (feature_has("Fix Blocks Not Breaking Instantly In Creative Mode", server_enabled)) {
        overwrite_calls(CreatorMode_stopDestroyBlock, CreatorMode_stopDestroyBlock_injection);
    }

    // Generate Caves
    if (feature_has("Generate Caves", server_generate_caves)) {
        overwrite_calls(RandomLevelSource_buildSurface, RandomLevelSource_buildSurface_injection);
    }

    // Tall Grass
    if (feature_has("Generate Tall Grass", server_generate_tall_grass)) {
        // Add Extra Iteration To Sugar-Cane Loop
        // First Loop Will Be Repurposed For Tall-Grass
        const unsigned char *instruction = (const unsigned char *) 0xb2e64;
        unsigned char extra_loop_patch[4] = {instruction[0], instruction[1], instruction[2], 0x5a}; // Change "bne" To "bpl"
        patch((void *) instruction, extra_loop_patch);
        // Generate
        overwrite_call((void *) 0xb229c, BiomeSource_getBiome, RandomLevelSource_postProcess_BiomeSource_getBiome_injection);
        overwrite_call((void *) 0xb2e54, ReedsFeature_place, RandomLevelSource_postProcess_ReedsFeature_place_injection);
        // Bone-Meal
        overwrite_call((void *) 0x93f6c, Level_getTile, DyePowderItem_useOn_Level_getTile_injection);
    }

    // Disable Hostile AI In Creative Mode
    if (feature_has("Disable Hostile AI In Creative Mode", server_enabled)) {
        overwrite_call((void *) 0x83b8c, PathfinderMob_findAttackTarget, PathfinderMob_updateAi_PathfinderMob_findAttackTarget_injection);
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
        overwrite_call((void *) 0x2e230, PaneCraftingScreen_recheckRecipes, PaneCraftingScreen_craftSelectedItem_PaneCraftingScreen_recheckRecipes_injection);
        overwrite_calls(Item_getCraftingRemainingItem, Item_getCraftingRemainingItem_injection);
    }

    // Display Date In Select World Screen
    if (feature_has("Display Date In Select World Screen", server_disabled)) {
        patch_vtable(AppPlatform_linux_getDateString, AppPlatform_linux_getDateString_injection);
    }

    // Fullscreen
    if (feature_has("Fullscreen Support", server_disabled)) {
        misc_run_on_key_press([](MCPI_UNUSED Minecraft *mc, int key) {
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
        overwrite_call_manual((void *) 0xc3b54, (void *) Tile_initTiles_std_string_constructor);
        overwrite_call_manual((void *) 0xc3c7c, (void *) Tile_initTiles_std_string_constructor);
        // Carried Tile Language Strings
        patch_address((void *) 0xc6674, (void *) "grassCarried");
        patch_address((void *) 0xc6684, (void *) "leavesCarried");
        // Invisible Bedrock Language String
        overwrite_call((void *) 0xc5f04, Tile_init, Tile_initTiles_Tile_init_invBedrock_injection);
    }

    // Prevent Pigmen From Burning In The Sun
    if (feature_has("Fix Pigmen Burning In The Sun", server_enabled)) {
        fix_pigmen_burning = true;
        overwrite_call((void *) 0x89a1c, Entity_getBrightness, Zombie_aiStep_getBrightness_injection);
    }

    // Fix Door Duplication
    if (feature_has("Fix Door Duplication", server_enabled)) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0xbe230, nop_patch);
        overwrite_call((void *) 0xbe110, DoorTile_spawnResources, DoorTile_neighborChanged_Tile_spawnResources_injection);
    }

    // Fix Cobweb Lighting
    if (feature_has("Fix Cobweb Lighting", server_enabled)) {
        overwrite_call((void *) 0xc444c, Tile_setLightBlock, Tile_initTiles_WebTile_setLightBlock_injection);
    }

    // Fix Fire Immunity
    if (feature_has("Fix Fire Immunity", server_enabled)) {
        overwrite_calls(Mob_baseTick, Mob_baseTick_injection_fire_immunity);
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
        overwrite_call((void *) 0xb198c, Level_getTopTile, Dimension_isValidSpawn_Level_getTopTile_injection);
    }

    // Fix Sugar Rendering
    if (feature_has("Fix Sugar Position In Hand", server_disabled)) {
        overwrite_call((void *) 0x976f8, Item_handEquipped, Item_initItems_Item_handEquipped_injection);
    }

    // Chest Placement
    if (feature_has("Remove Chest Placement Restrictions", server_enabled)) {
        overwrite_calls(ChestTile_mayPlace, ChestTile_mayPlace_injection);
        unsigned char skip_neighbor_check_patch[4] = {0x01, 0x30, 0xa0, 0xe3}; // "mov r3, #0x1"
        patch((void *) 0xcfaf8, skip_neighbor_check_patch);
    }

    // Fix TextEditScreen Crashing After Sign Is Destroyed
    if (feature_has("Close Editor When Sign Is Destroyed", server_disabled)) {
        overwrite_calls(Minecraft_setScreen, Minecraft_setScreen_injection);
        overwrite_calls(SignTileEntity_destructor_complete, SignTileEntity_destructor_injection);
    }

    // Fix Hanging When Spawning
    if (feature_has("Fix Hanging When No Valid Spawn Point Exists", server_enabled)) {
        overwrite_calls(Level_setInitialSpawn, Level_setInitialSpawn_or_validateSpawn_injection);
        overwrite_calls(Level_validateSpawn, Level_setInitialSpawn_or_validateSpawn_injection);
        overwrite_calls(Level_getTopTile, Level_getTopTile_injection);
    }

    // Fix Torch Placement
    if (feature_has("Fix Torch Placement", server_enabled)) {
        overwrite_calls(TorchTile_onPlace, TorchTile_onPlace_injection);
        overwrite_calls(TorchTile_getPlacedOnFaceDataValue, TorchTile_getPlacedOnFaceDataValue_injection);
    }

    // Fix Egg Behavior
    if (feature_has("Fix Eggs Spawning Abnormally Healthy Chickens", server_enabled)) {
        overwrite_call((void *) 0x7de0c, Chicken_moveTo, ThrownEgg_onHit_Chicken_moveTo_injection);
    }

    // Fix Doors And Trapdoors Closing When Updated
    if (feature_has("Stop Doors And Trapdoors Closing When Updated", server_enabled)) {
        // Patch out the `setOpen` call for doors and trap doors in neighborChanged
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0xbe1a0, nop_patch); // door
        patch((void *) 0xcf818, nop_patch); // trapdoor
    }

    // Fix Dropping Armor On Death
    if (feature_has("Fix Dropping Armor On Death", server_enabled)) {
        overwrite_calls(LocalPlayer_die, LocalPlayer_die_injection);
    }

    // Reliable Chunk Saving
    if (feature_has("Sync Chunks To Disk After Saving", server_enabled)) {
        overwrite_calls(RegionFile_writeChunk, RegionFile_writeChunk_injection);
    }

    // Force Saving On Exiting World
    if (feature_has("Force Saving Level Data On World Exit", server_enabled)) {
        overwrite_calls(Minecraft_leaveGame, Minecraft_leaveGame_injection);
    }

    // Disable overwrite_calls() Before NinecraftApp::init
    overwrite_calls(NinecraftApp_init, [](NinecraftApp_init_t original, NinecraftApp *self) {
        thunk_enabler = [](MCPI_UNUSED void *a, MCPI_UNUSED void *b) -> void * {
            IMPOSSIBLE();
        };
        can_construct_custom_wrappers = true;
        // Call Original Method
        original(self);
    });

    // Init Other Components
    _init_misc_logging();
    _init_misc_ui();
    _init_misc_api();
    _init_misc_graphics();
    _init_misc_home();
    _init_misc_item_rendering();
    _init_misc_daynight_cycle();
}
