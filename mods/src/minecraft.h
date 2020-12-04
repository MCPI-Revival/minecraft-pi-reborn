#ifndef MINECRAFT_H

#define MINECRAFT_H

#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// Globals

static char **default_username = (char **) 0x18fd4;

static unsigned char **Item_flintAndSteel = (unsigned char **) 0x17ba70;
static unsigned char **Item_snowball = (unsigned char **) 0x17bbb0;
static unsigned char **Item_shears = (unsigned char **) 0x17bbf0;
static unsigned char **Item_egg = (unsigned char **) 0x17bbd0;
static unsigned char **Item_dye_powder = (unsigned char **) 0x17bbe0;
static unsigned char **Item_camera = (unsigned char **) 0x17bc14;

static unsigned char **Tile_water = (unsigned char **) 0x181b3c;
static unsigned char **Tile_lava = (unsigned char **) 0x181cc8;
static unsigned char **Tile_calmWater = (unsigned char **) 0x181b40;
static unsigned char **Tile_calmLava = (unsigned char **) 0x181ccc;
static unsigned char **Tile_glowingObsidian = (unsigned char **) 0x181dcc;
static unsigned char **Tile_web = (unsigned char **) 0x181d08;
static unsigned char **Tile_topSnow = (unsigned char **) 0x181b30;
static unsigned char **Tile_ice = (unsigned char **) 0x181d80;
static unsigned char **Tile_invisible_bedrock = (unsigned char **) 0x181d94;

static unsigned char **Tile_leaves = (unsigned char **) 0x18120c;
static unsigned char **Tile_leaves_carried = (unsigned char **) 0x181dd8;
static unsigned char **Tile_grass = (unsigned char **) 0x181b14;
static unsigned char **Tile_grass_carried = (unsigned char **) 0x181dd4;

static float *InvGuiScale = (float *) 0x135d98;

typedef long int (*getRemainingFileSize_t)(FILE *file);
static getRemainingFileSize_t getRemainingFileSize = (getRemainingFileSize_t) 0xba520;

// Structures

struct LevelSettings {
    unsigned long seed;
    int32_t game_type;
};

struct RakNet_RakNetGUID {
    unsigned char data[10];
};

struct RakNet_SystemAddress {
    unsigned char data[20];
};

struct RakNet_BitStream {
    unsigned char data[273];
};

struct RakDataOutput {
    unsigned char data[8];
};

struct RakDataInput {
    unsigned char data[8];
};

// GameMode

typedef void (*GameMode_releaseUsingItem_t)(unsigned char *game_mode, unsigned char *player);

// Minecraft

typedef void (*Minecraft_init_t)(unsigned char *minecraft);
static Minecraft_init_t Minecraft_init = (Minecraft_init_t) 0x1700c;

typedef void (*Minecraft_tickInput_t)(unsigned char *minecraft);
static Minecraft_tickInput_t Minecraft_tickInput = (Minecraft_tickInput_t) 0x15ffc;

typedef void (*Minecraft_setIsCreativeMode_t)(unsigned char *, int32_t);
static Minecraft_setIsCreativeMode_t Minecraft_setIsCreativeMode = (Minecraft_setIsCreativeMode_t) 0x16ec4;

typedef int32_t (*Minecraft_isTouchscreen_t)(unsigned char *minecraft);
static Minecraft_isTouchscreen_t Minecraft_isTouchscreen = (Minecraft_isTouchscreen_t) 0x1639c;

typedef void (*Minecraft_setScreen_t)(unsigned char *minecraft, unsigned char *screen);
static Minecraft_setScreen_t Minecraft_setScreen = (Minecraft_setScreen_t) 0x15d6c;

typedef void (*Minecraft_tick_t)(unsigned char *minecraft, int32_t param_1, int32_t param_2);
static Minecraft_tick_t Minecraft_tick = (Minecraft_tick_t) 0x16934;

typedef void (*Minecraft_update_t)(unsigned char *minecraft);
static Minecraft_update_t Minecraft_update = (Minecraft_update_t) 0x16b74;

typedef void (*Minecraft_hostMultiplayer_t)(unsigned char *minecraft, int32_t port);
static Minecraft_hostMultiplayer_t Minecraft_hostMultiplayer = (Minecraft_hostMultiplayer_t) 0x16664;

typedef const char *(*Minecraft_getProgressMessage_t)(unsigned char *minecraft);
static Minecraft_getProgressMessage_t Minecraft_getProgressMessage = (Minecraft_getProgressMessage_t) 0x16e58;

typedef uint32_t (*Minecraft_isLevelGenerated_t)(unsigned char *minecraft);
static Minecraft_isLevelGenerated_t Minecraft_isLevelGenerated = (Minecraft_isLevelGenerated_t) 0x16e6c;

typedef int32_t (*Minecraft_isCreativeMode_t)(unsigned char *minecraft);
static Minecraft_isCreativeMode_t Minecraft_isCreativeMode = (Minecraft_isCreativeMode_t) 0x17270;

// MouseBuildInput

typedef int32_t (*MouseBuildInput_tickBuild_t)(unsigned char *mouse_build_input, unsigned char *player, uint32_t *build_action_intention_return);
static MouseBuildInput_tickBuild_t MouseBuildInput_tickBuild = (MouseBuildInput_tickBuild_t) 0x17c98;
static void *MouseBuildInput_tickBuild_vtable_addr = (void *) 0x102564;

// Player

typedef int (*Player_isUsingItem_t)(unsigned char *player);
static Player_isUsingItem_t Player_isUsingItem = (Player_isUsingItem_t) 0x8f15c;

typedef void (*Player_setArmor_t)(unsigned char *player, int32_t slot, unsigned char *item);
static Player_setArmor_t Player_setArmor = (Player_setArmor_t) 0x8fde0;

// Player

static void *LocalPlayer_openTextEdit_vtable_addr = (void *) 0x106460;

// Gui

typedef void (*Gui_tick_t)(unsigned char *gui);
static Gui_tick_t Gui_tick = (Gui_tick_t) 0x27778;

typedef void (*Gui_handleClick_t)(unsigned char *gui, int32_t param_2, int32_t param_3, int32_t param_4);
static Gui_handleClick_t Gui_handleClick = (Gui_handleClick_t) 0x2599c;

typedef void (*Gui_renderOnSelectItemNameText_t)(unsigned char *gui, int32_t param_1, unsigned char *font, int32_t param_2);
static Gui_renderOnSelectItemNameText_t Gui_renderOnSelectItemNameText = (Gui_renderOnSelectItemNameText_t) 0x26aec;

// GameMode Constructors

static void *Creator = (void *) 0x1a044;
static void *SurvivalMode = (void *) 0x1b7d8;
static void *CreativeMode = (void *) 0x1b258;

// LevelData

typedef uint32_t (*LevelData_getSpawnMobs_t)(unsigned char *level_data);
static LevelData_getSpawnMobs_t LevelData_getSpawnMobs = (LevelData_getSpawnMobs_t) 0xbabec;

// Level

typedef void (*Level_saveLevelData_t)(unsigned char *level);
static Level_saveLevelData_t Level_saveLevelData = (Level_saveLevelData_t) 0xa2e94;

// TextEditScreen

typedef unsigned char *(*TextEditScreen_t)(unsigned char *text_edit_screen, unsigned char *sign);
static TextEditScreen_t TextEditScreen = (TextEditScreen_t) 0x3a840;

static void *TextEditScreen_updateEvents_vtable_addr = (void *) 0x10531c;

// ProgressScreen

typedef void *(*ProgressScreen_t)(unsigned char *obj);
static ProgressScreen_t ProgressScreen = (ProgressScreen_t) 0x37044;

// Screen

typedef void (*Screen_updateEvents_t)(unsigned char *screen);
static Screen_updateEvents_t Screen_updateEvents = (Screen_updateEvents_t) 0x28eb8;

typedef void (*Screen_keyboardNewChar_t)(unsigned char *screen, char key);
typedef void (*Screen_keyPressed_t)(unsigned char *screen, int32_t key);

typedef void (*Screen_tick_t)(unsigned char *screen);

// SelectWorldScreen

static Screen_tick_t SelectWorldScreen_tick = (Screen_tick_t) 0x38a2c;
static void *SelectWorldScreen_tick_vtable_addr = (void *) 0x104f78;

// Touch::SelectWorldScreen

static Screen_tick_t Touch_SelectWorldScreen_tick = (Screen_tick_t) 0x3d96c;
static void *Touch_SelectWorldScreen_tick_vtable_addr = (void *) 0x105780;

// ItemInstance

#define ITEM_INSTANCE_SIZE 0xc

typedef unsigned char *(*ItemInstance_constructor_t)(unsigned char *item_instance, unsigned char *item);
static ItemInstance_constructor_t ItemInstance_constructor_item = (ItemInstance_constructor_t) 0x9992c;
static ItemInstance_constructor_t ItemInstance_constructor_tile = (ItemInstance_constructor_t) 0x998e4;

typedef unsigned char *(*ItemInstance_constructor_extra_t)(unsigned char *item_instance, unsigned char *item, int32_t count, int32_t auxilary);
static ItemInstance_constructor_extra_t ItemInstance_constructor_title_extra = (ItemInstance_constructor_extra_t) 0x99918;
static ItemInstance_constructor_extra_t ItemInstance_constructor_item_extra = (ItemInstance_constructor_extra_t) 0x99960;

// FillingContainer

typedef int32_t (*FillingContainer_addItem_t)(unsigned char *filling_container, unsigned char *item_instance);
static FillingContainer_addItem_t FillingContainer_addItem = (FillingContainer_addItem_t) 0x92aa0;

// RakNet::RakPeer

typedef struct RakNet_SystemAddress (*RakNet_RakPeer_GetSystemAddressFromGuid_t)(unsigned char *rak_peer, struct RakNet_RakNetGUID guid);

// RakNet::BitStream

typedef unsigned char *(*RakNet_BitStream_constructor_t)(struct RakNet_BitStream *stream);
static RakNet_BitStream_constructor_t RakNet_BitStream_constructor = (RakNet_BitStream_constructor_t) 0xd3b84;

typedef void (*RakNet_BitStream_destructor_t)(struct RakNet_BitStream *stream);
static RakNet_BitStream_destructor_t RakNet_BitStream_destructor = (RakNet_BitStream_destructor_t) 0xd3ce8;

// RakDataOutput

static unsigned char *RakDataOutput_vtable = (unsigned char *) 0x109628;

// RakDataInput

static unsigned char *RakDataInput_vtable = (unsigned char *) 0x1095c8;

// ServerSideNetworkHandler

typedef void (*ServerSideNetworkHandler_onDisconnect_t)(unsigned char *server_side_network_handler, unsigned char *guid);
static ServerSideNetworkHandler_onDisconnect_t ServerSideNetworkHandler_onDisconnect = (ServerSideNetworkHandler_onDisconnect_t) 0x75164;
static void *ServerSideNetworkHandler_onDisconnect_vtable_addr = (void *) 0x109bb0;

typedef unsigned char *(*ServerSideNetworkHandler_getPlayer_t)(unsigned char *server_side_network_handler, unsigned char *guid);
static ServerSideNetworkHandler_getPlayer_t ServerSideNetworkHandler_getPlayer = (ServerSideNetworkHandler_getPlayer_t) 0x75464;

// CompoundTag

typedef unsigned char *(*CompoundTag_t)(unsigned char *tag);
static CompoundTag_t CompoundTag = (CompoundTag_t) 0xb9920;

// Tag

typedef void (*Tag_writeNamedTag_t)(unsigned char *tag, struct RakDataOutput *output);
static Tag_writeNamedTag_t Tag_writeNamedTag = (Tag_writeNamedTag_t) 0x6850c;

typedef void (*Tag_deleteChildren_t)(unsigned char *tag);
typedef void (*Tag_destructor_t)(unsigned char *tag);

// Entity

typedef void (*Entity_saveWithoutId_t)(unsigned char *entity, unsigned char *tag);

typedef void (*Entity_load_t)(unsigned char *entity, unsigned char *tag);

typedef void (*Entity_moveTo_t)(unsigned char *entity, float param_1, float param_2, float param_3, float param_4, float param_5);
static Entity_moveTo_t Entity_moveTo = (Entity_moveTo_t) 0x7a834;

typedef void (*Entity_die_t)(unsigned char *entity, unsigned char *cause);

// ServerPlayer

static void *ServerPlayer_moveTo_vtable_addr = (void *) 0x109e54;

// NbtIo

typedef unsigned char *(*NbtIo_read_t)(struct RakDataInput *input);
static NbtIo_read_t NbtIo_read = (NbtIo_read_t) 0xb98cc;

// Inventory

typedef void (*Inventory_clearInventoryWithDefault_t)(unsigned char *inventory);
static Inventory_clearInventoryWithDefault_t Inventory_clearInventoryWithDefault = (Inventory_clearInventoryWithDefault_t) 0x8e7c8;

typedef void (*Inventory_selectSlot_t)(unsigned char *inventory, int32_t slot);
static Inventory_selectSlot_t Inventory_selectSlot = (Inventory_selectSlot_t) 0x8d13c;

// TripodCameraRenderer

typedef unsigned char *(*TripodCameraRenderer_t)(unsigned char *renderer);
static TripodCameraRenderer_t TripodCameraRenderer = (TripodCameraRenderer_t) 0x6583c;

// EntityRenderDispatcher

typedef unsigned char *(*EntityRenderDispatcher_t)(unsigned char *dispatcher);
static EntityRenderDispatcher_t EntityRenderDispatcher = (EntityRenderDispatcher_t) 0x6096c;

typedef void (*EntityRenderDispatcher_assign_t)(unsigned char *dispatcher, unsigned char entity_id, unsigned char *renderer);
static EntityRenderDispatcher_assign_t EntityRenderDispatcher_assign = (EntityRenderDispatcher_assign_t) 0x6094c;

// ItemRenderer

typedef float (*ItemRenderer_renderGuiItemCorrect_t)(unsigned char *font, unsigned char *textures, unsigned char *item_instance, int32_t param_1, int32_t param_2);
static ItemRenderer_renderGuiItemCorrect_t ItemRenderer_renderGuiItemCorrect = (ItemRenderer_renderGuiItemCorrect_t) 0x639a0;

// Method That Require C++ Types
#ifdef __cplusplus

#include <string>

// AppPlatform

typedef void (*AppPlatform_saveScreenshot_t)(unsigned char *app_platform, std::string const& param1, std::string const& param_2);
static void *AppPlatform_linux_saveScreenshot_vtable_addr = (void *) 0x102160;

struct AppPlatform_readAssetFile_return_value {
    char *data;
    int32_t length;
};
typedef AppPlatform_readAssetFile_return_value (*AppPlatform_readAssetFile_t)(unsigned char *app_platform, std::string const& path);
static AppPlatform_readAssetFile_t AppPlatform_readAssetFile = (AppPlatform_readAssetFile_t) 0x12b10;

// Minecraft

typedef void (*Minecraft_selectLevel_t)(unsigned char *minecraft, std::string const& level_dir, std::string const& level_name, LevelSettings const& vsettings);
static Minecraft_selectLevel_t Minecraft_selectLevel = (Minecraft_selectLevel_t) 0x16f38;

typedef void (*Minecraft_leaveGame_t)(unsigned char *minecraft, bool save_remote_level);
static Minecraft_leaveGame_t Minecraft_leaveGame = (Minecraft_leaveGame_t) 0x15ea0;

// Level

typedef void (*Level_addParticle_t)(unsigned char *level, std::string const& particle, float x, float y, float z, float deltaX, float deltaY, float deltaZ, int count);
static Level_addParticle_t Level_addParticle = (Level_addParticle_t) 0xa449c;

// Gui

typedef void (*Gui_addMessage_t)(unsigned char *gui, std::string const& text);
static Gui_addMessage_t Gui_addMessage = (Gui_addMessage_t) 0x27820;

typedef void (*Gui_renderChatMessages_t)(unsigned char *gui, int32_t param_1, uint32_t param_2, bool param_3, unsigned char *font);
static Gui_renderChatMessages_t Gui_renderChatMessages = (Gui_renderChatMessages_t) 0x273d8;

// Textures

typedef void (*Textures_tick_t)(unsigned char *textures, bool param_1);
static Textures_tick_t Textures_tick = (Textures_tick_t) 0x531c4;

// RakNet::RakPeer

typedef bool (*RakNet_RakPeer_IsBanned_t)(unsigned char *rakpeer, const char *ip);
static RakNet_RakPeer_IsBanned_t RakNet_RakPeer_IsBanned = (RakNet_RakPeer_IsBanned_t) 0xda3b4;

// RakNet::BitStream

typedef struct RakNet_BitStream *(*RakNet_BitStream_constructor_with_data_t)(struct RakNet_BitStream *stream, unsigned char *data, uint32_t size, bool copyData);
static RakNet_BitStream_constructor_with_data_t RakNet_BitStream_constructor_with_data = (RakNet_BitStream_constructor_with_data_t) 0xd3c30;

// RakNet::SystemAddress

typedef char *(*RakNet_SystemAddress_ToString_t)(struct RakNet_SystemAddress *system_address, bool print_delimiter, char delimiter);
static RakNet_SystemAddress_ToString_t RakNet_SystemAddress_ToString = (RakNet_SystemAddress_ToString_t) 0xd6198;

// ServerSideNetworkHandler

typedef void (*ServerSideNetworkHandler_displayGameMessage_t)(unsigned char *server_side_network_handler, std::string const& message);
static ServerSideNetworkHandler_displayGameMessage_t ServerSideNetworkHandler_displayGameMessage = (ServerSideNetworkHandler_displayGameMessage_t) 0x750c4;

// SimpleChooseLevelScreen

typedef unsigned char *(*SimpleChooseLevelScreen_t)(unsigned char *simple_choose_level_screen, std::string const& world_name);
static SimpleChooseLevelScreen_t SimpleChooseLevelScreen = (SimpleChooseLevelScreen_t) 0x31404;

// SelectWorldScreen

typedef std::string &(*SelectWorldScreen_getUniqueLevelName_t)(std::string &new_name, unsigned char *screen, std::string const& name);
static SelectWorldScreen_getUniqueLevelName_t SelectWorldScreen_getUniqueLevelName = (SelectWorldScreen_getUniqueLevelName_t) 0x388ec;

// Touch::SelectWorldScreen

static SelectWorldScreen_getUniqueLevelName_t Touch_SelectWorldScreen_getUniqueLevelName = (SelectWorldScreen_getUniqueLevelName_t) 0x3d82c;

#endif

#pragma GCC diagnostic pop

#endif