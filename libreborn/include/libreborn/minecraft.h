#pragma once

#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// bool In C
#ifndef __cplusplus
typedef uint32_t bool;
#endif

// Globals

static char **default_path = (char **) 0xe264; // /.minecraft/
static char **default_username = (char **) 0x18fd4; // StevePi

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

// Tile

static uint32_t Tile_id_property_offset = 0x8;

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

// GameMode

typedef void (*GameMode_releaseUsingItem_t)(unsigned char *game_mode, unsigned char *player);
static uint32_t GameMode_releaseUsingItem_vtable_offset = 0x5c;

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

typedef bool (*Minecraft_isCreativeMode_t)(unsigned char *minecraft);
static Minecraft_isCreativeMode_t Minecraft_isCreativeMode = (Minecraft_isCreativeMode_t) 0x17270;

typedef void (*Minecraft_releaseMouse_t)(unsigned char *minecraft);
static Minecraft_releaseMouse_t Minecraft_releaseMouse = (Minecraft_releaseMouse_t) 0x15d30;

typedef void (*Minecraft_grabMouse_t)(unsigned char *minecraft);
static Minecraft_grabMouse_t Minecraft_grabMouse = (Minecraft_grabMouse_t) 0x15d10;

typedef void (*Minecraft_leaveGame_t)(unsigned char *minecraft, bool save_remote_level);
static Minecraft_leaveGame_t Minecraft_leaveGame = (Minecraft_leaveGame_t) 0x15ea0;

static uint32_t Minecraft_screen_width_property_offset = 0x20; // int32_t
static uint32_t Minecraft_network_handler_property_offset = 0x174; // NetEventCallback *
static uint32_t Minecraft_rak_net_instance_property_offset = 0x170; // RakNetInstance *
static uint32_t Minecraft_level_property_offset = 0x188; // Level *
static uint32_t Minecraft_textures_property_offset = 0x164; // Textures *
static uint32_t Minecraft_game_mode_property_offset = 0x160; // GameMode *
static uint32_t Minecraft_player_property_offset = 0x18c; // LocalPlayer *
static uint32_t Minecraft_options_property_offset = 0x3c; // Options
static uint32_t Minecraft_hit_result_property_offset = 0xc38; // HitResult
static uint32_t Minecraft_progress_property_offset = 0xc60; // int32_t
static uint32_t Minecraft_command_server_property_offset = 0xcc0; // CommandServer *
static uint32_t Minecraft_screen_property_offset = 0xc10; // Screen *
static uint32_t Minecraft_gui_property_offset = 0x198; // Gui

// CommandServer

static uint32_t CommandServer_minecraft_property_offset = 0x18; // Minecraft *

// Packet

typedef void (*Packet_read_t)(unsigned char *packet, unsigned char *bit_stream);

// LoginPacket

static Packet_read_t LoginPacket_read = (Packet_read_t) 0x6e5f8;
static void *LoginPacket_read_vtable_addr = (void *) 0x108dcc;

static uint32_t LoginPacket_username_property_offset = 0xc; // RakString

// ChatPacket

static uint32_t ChatPacket_message_property_offset = 0xc; // char *

// HitResult

static uint32_t HitResult_type_property_offset = 0x0;

// Options

static uint32_t Options_fancy_graphics_property_offset = 0x17; // unsigned char / bool
static uint32_t Options_split_controls_property_offset = 0x105; // int32_t
static uint32_t Options_peaceful_mode_property_offset = 0xe8; // unsigned char / bool
static uint32_t Options_3d_anaglyph_property_offset = 0x15; // unsigned char / bool
static uint32_t Options_ambient_occlusion_property_offset = 0x18; // unsigned char / bool
static uint32_t Options_hide_gui_property_offset = 0xec; // unsigned char / bool
static uint32_t Options_third_person_property_offset = 0xed; // unsigned char / bool
static uint32_t Options_render_distance_property_offset = 0x10; // int32_t

// MouseBuildInput

typedef int32_t (*MouseBuildInput_tickBuild_t)(unsigned char *mouse_build_input, unsigned char *player, uint32_t *build_action_intention_return);
static MouseBuildInput_tickBuild_t MouseBuildInput_tickBuild = (MouseBuildInput_tickBuild_t) 0x17c98;
static void *MouseBuildInput_tickBuild_vtable_addr = (void *) 0x102564;

// Player

typedef int (*Player_isUsingItem_t)(unsigned char *player);
static Player_isUsingItem_t Player_isUsingItem = (Player_isUsingItem_t) 0x8f15c;

static uint32_t Player_username_property_offset = 0xbf4; // char *

// Entity

typedef void (*Entity_die_t)(unsigned char *entity, unsigned char *cause);
static uint32_t Entity_die_vtable_offset = 0x130;

// Mob

typedef void (*Mob_actuallyHurt_t)(unsigned char *entity, int32_t damage);
static Mob_actuallyHurt_t Mob_actuallyHurt = (Mob_actuallyHurt_t) 0x7f11c;

static uint32_t Mob_health_property_offset = 0xec; // int32_t

// LocalPlayer

static Mob_actuallyHurt_t LocalPlayer_actuallyHurt = (Mob_actuallyHurt_t) 0x44010;
static void *LocalPlayer_actuallyHurt_vtable_addr = (void *) 0x10639c;

static void *LocalPlayer_openTextEdit_vtable_addr = (void *) 0x106460;

static uint32_t LocalPlayer_minecraft_property_offset = 0xc90; // Minecraft *

// ServerPlayer

static void *ServerPlayer_actuallyHurt_vtable_addr = (void *) 0x109fa4;

static uint32_t ServerPlayer_minecraft_property_offset = 0xc8c; // Minecraft *
static uint32_t ServerPlayer_guid_property_offset = 0xc08; // RakNetGUID

// Gui

typedef void (*Gui_tick_t)(unsigned char *gui);
static Gui_tick_t Gui_tick = (Gui_tick_t) 0x27778;

typedef void (*Gui_handleClick_t)(unsigned char *gui, int32_t param_2, int32_t param_3, int32_t param_4);
static Gui_handleClick_t Gui_handleClick = (Gui_handleClick_t) 0x2599c;

typedef void (*Gui_renderOnSelectItemNameText_t)(unsigned char *gui, int32_t param_1, unsigned char *font, int32_t param_2);
static Gui_renderOnSelectItemNameText_t Gui_renderOnSelectItemNameText = (Gui_renderOnSelectItemNameText_t) 0x26aec;

typedef void (*Gui_renderChatMessages_t)(unsigned char *gui, int32_t param_1, uint32_t param_2, bool param_3, unsigned char *font);
static Gui_renderChatMessages_t Gui_renderChatMessages = (Gui_renderChatMessages_t) 0x273d8;

static uint32_t Gui_minecraft_property_offset = 0x9f4; // Minecraft *
static uint32_t Gui_selected_item_text_timer_property_offset = 0x9fc; // float

// Textures

typedef void (*Textures_tick_t)(unsigned char *textures, bool param_1);
static Textures_tick_t Textures_tick = (Textures_tick_t) 0x531c4;

// GameMode Constructors

#define CREATOR_MODE_SIZE 0x18
static void *CreatorMode = (void *) 0x1a044;
#define SURVIVAL_MODE_SIZE 0x24
static void *SurvivalMode = (void *) 0x1b7d8;

// LevelData

typedef uint32_t (*LevelData_getSpawnMobs_t)(unsigned char *level_data);
static LevelData_getSpawnMobs_t LevelData_getSpawnMobs = (LevelData_getSpawnMobs_t) 0xbabec;

// Level

typedef void (*Level_saveLevelData_t)(unsigned char *level);
static Level_saveLevelData_t Level_saveLevelData = (Level_saveLevelData_t) 0xa2e94;

static uint32_t Level_players_property_offset = 0x60; // std::vector<ServerPlayer *>

// TextEditScreen

#define TEXT_EDIT_SCREEN_SIZE 0xd0

typedef unsigned char *(*TextEditScreen_t)(unsigned char *text_edit_screen, unsigned char *sign);
static TextEditScreen_t TextEditScreen = (TextEditScreen_t) 0x3a840;

static void *TextEditScreen_updateEvents_vtable_addr = (void *) 0x10531c;

// ProgressScreen

#define PROGRESS_SCREEN_SIZE 0x4c

typedef void *(*ProgressScreen_t)(unsigned char *obj);
static ProgressScreen_t ProgressScreen = (ProgressScreen_t) 0x37044;

// Screen

typedef void (*Screen_updateEvents_t)(unsigned char *screen);
static Screen_updateEvents_t Screen_updateEvents = (Screen_updateEvents_t) 0x28eb8;

typedef void (*Screen_keyboardNewChar_t)(unsigned char *screen, char key);
static uint32_t Screen_keyboardNewChar_vtable_offset = 0x70;

typedef void (*Screen_keyPressed_t)(unsigned char *screen, int32_t key);
static uint32_t Screen_keyPressed_vtable_offset = 0x6c;

typedef void (*Screen_tick_t)(unsigned char *screen);

static uint32_t Screen_minecraft_property_offset = 0x14; // Minecraft *

// SelectWorldScreen

static Screen_tick_t SelectWorldScreen_tick = (Screen_tick_t) 0x38a2c;
static void *SelectWorldScreen_tick_vtable_addr = (void *) 0x104f78;

static uint32_t SelectWorldScreen_should_create_world_property_offset = 0xfc; // bool
static uint32_t SelectWorldScreen_world_created_property_offset = 0xf9; // bool

// Touch::SelectWorldScreen

static Screen_tick_t Touch_SelectWorldScreen_tick = (Screen_tick_t) 0x3d96c;
static void *Touch_SelectWorldScreen_tick_vtable_addr = (void *) 0x105780;

static uint32_t Touch_SelectWorldScreen_should_create_world_property_offset = 0x154; // bool
static uint32_t Touch_SelectWorldScreen_world_created_property_offset = 0x151; // bool

// ItemInstance

#define ITEM_INSTANCE_SIZE 0xc

typedef unsigned char *(*ItemInstance_constructor_t)(unsigned char *item_instance, unsigned char *item);
static ItemInstance_constructor_t ItemInstance_constructor_item = (ItemInstance_constructor_t) 0x9992c;
static ItemInstance_constructor_t ItemInstance_constructor_tile = (ItemInstance_constructor_t) 0x998e4;

typedef unsigned char *(*ItemInstance_constructor_extra_t)(unsigned char *item_instance, unsigned char *item, int32_t count, int32_t auxilary);
static ItemInstance_constructor_extra_t ItemInstance_constructor_tile_extra = (ItemInstance_constructor_extra_t) 0x99918;
static ItemInstance_constructor_extra_t ItemInstance_constructor_item_extra = (ItemInstance_constructor_extra_t) 0x99960;

static uint32_t ItemInstance_count_property_offset = 0x0;
static uint32_t ItemInstance_id_property_offset = 0x4;
static uint32_t ItemInstance_auxilary_property_offset = 0x8;

// FillingContainer

typedef int32_t (*FillingContainer_addItem_t)(unsigned char *filling_container, unsigned char *item_instance);
static FillingContainer_addItem_t FillingContainer_addItem = (FillingContainer_addItem_t) 0x92aa0;

// RakNet::RakString

typedef void (*RakNet_RakString_Assign_t)(unsigned char *rak_string, const char *str);
static RakNet_RakString_Assign_t RakNet_RakString_Assign = (RakNet_RakString_Assign_t) 0xe9e34;

static uint32_t RakNet_RakString_sharedString_property_offset = 0x0; // RakNet::RakString::SharedString *

// RakNet::RakString::SharedString

static uint32_t RakNet_RakString_SharedString_c_str_property_offset = 0x10; // char *

// RakNetInstance

typedef void (*RakNetInstance_send_t)(unsigned char *rak_net_instance, unsigned char *packet);
static uint32_t RakNetInstance_send_vtable_offset = 0x38;

typedef uint32_t (*RakNetInstance_isServer_t)(unsigned char *rak_net_instance);
static uint32_t RakNetInstance_isServer_vtable_offset = 0x48;

static uint32_t RakNetInstance_peer_property_offset = 0x4; // RakNet::RakPeer *

// RakNet::RakPeer

typedef struct RakNet_SystemAddress (*RakNet_RakPeer_GetSystemAddressFromGuid_t)(unsigned char *rak_peer, struct RakNet_RakNetGUID guid);
static uint32_t RakNet_RakPeer_GetSystemAddressFromGuid_vtable_offset = 0xd0;

typedef bool (*RakNet_RakPeer_IsBanned_t)(unsigned char *rak_peer, const char *ip);
static RakNet_RakPeer_IsBanned_t RakNet_RakPeer_IsBanned = (RakNet_RakPeer_IsBanned_t) 0xda3b4;

// RakNet::SystemAddress

typedef char *(*RakNet_SystemAddress_ToString_t)(struct RakNet_SystemAddress *system_address, bool print_delimiter, char delimiter);
static RakNet_SystemAddress_ToString_t RakNet_SystemAddress_ToString = (RakNet_SystemAddress_ToString_t) 0xd6198;

// ServerSideNetworkHandler

typedef void (*ServerSideNetworkHandler_onDisconnect_t)(unsigned char *server_side_network_handler, unsigned char *guid);
static ServerSideNetworkHandler_onDisconnect_t ServerSideNetworkHandler_onDisconnect = (ServerSideNetworkHandler_onDisconnect_t) 0x75164;
static void *ServerSideNetworkHandler_onDisconnect_vtable_addr = (void *) 0x109bb0;

typedef unsigned char *(*ServerSideNetworkHandler_getPlayer_t)(unsigned char *server_side_network_handler, unsigned char *guid);
static ServerSideNetworkHandler_getPlayer_t ServerSideNetworkHandler_getPlayer = (ServerSideNetworkHandler_getPlayer_t) 0x75464;

typedef void (*ServerSideNetworkHandler_handle_t)(unsigned char *server_side_network_handler, unsigned char *rak_net_guid, unsigned char *packet);

static void *ServerSideNetworkHandler_handle_ChatPacket_vtable_addr = (void *) 0x109c60;

// Inventory

typedef void (*Inventory_selectSlot_t)(unsigned char *inventory, int32_t slot);
static Inventory_selectSlot_t Inventory_selectSlot = (Inventory_selectSlot_t) 0x8d13c;

// TripodCameraRenderer

#define TRIPOD_CAMERA_RENDERER_SIZE 0x193

typedef unsigned char *(*TripodCameraRenderer_t)(unsigned char *renderer);
static TripodCameraRenderer_t TripodCameraRenderer = (TripodCameraRenderer_t) 0x6583c;

// EntityRenderDispatcher

typedef unsigned char *(*EntityRenderDispatcher_t)(unsigned char *dispatcher);
static EntityRenderDispatcher_t EntityRenderDispatcher = (EntityRenderDispatcher_t) 0x6096c;

typedef void (*EntityRenderDispatcher_assign_t)(unsigned char *dispatcher, unsigned char entity_id, unsigned char *renderer);
static EntityRenderDispatcher_assign_t EntityRenderDispatcher_assign = (EntityRenderDispatcher_assign_t) 0x6094c;

// TileEntity

static uint32_t TileEntity_id_property_offset = 0x18; // int32_t

// ItemRenderer

typedef float (*ItemRenderer_renderGuiItemCorrect_t)(unsigned char *font, unsigned char *textures, unsigned char *item_instance, int32_t param_1, int32_t param_2);
static ItemRenderer_renderGuiItemCorrect_t ItemRenderer_renderGuiItemCorrect = (ItemRenderer_renderGuiItemCorrect_t) 0x639a0;

// Method That Require C++ Types
#ifdef __cplusplus

#include <string>

// Structures

struct AppPlatform_readAssetFile_return_value {
    char *data;
    int32_t length;
};

struct ConnectedClient {
    uint32_t sock;
    std::string str;
    long time;
};

// AppPlatform

typedef void (*AppPlatform_saveScreenshot_t)(unsigned char *app_platform, std::string const& param1, std::string const& param_2);
static void *AppPlatform_linux_saveScreenshot_vtable_addr = (void *) 0x102160;

typedef AppPlatform_readAssetFile_return_value (*AppPlatform_readAssetFile_t)(unsigned char *app_platform, std::string const& path);
static AppPlatform_readAssetFile_t AppPlatform_readAssetFile = (AppPlatform_readAssetFile_t) 0x12b10;

// Minecraft

typedef void (*Minecraft_selectLevel_t)(unsigned char *minecraft, std::string const& level_dir, std::string const& level_name, LevelSettings const& vsettings);
static Minecraft_selectLevel_t Minecraft_selectLevel = (Minecraft_selectLevel_t) 0x16f38;

// CommandServer

typedef std::string (*CommandServer_parse_t)(unsigned char *command_server, struct ConnectedClient &client, std::string const& command);
static CommandServer_parse_t CommandServer_parse = (CommandServer_parse_t) 0x6aa8c;

// Level

typedef void (*Level_addParticle_t)(unsigned char *level, std::string const& particle, float x, float y, float z, float deltaX, float deltaY, float deltaZ, int count);
static Level_addParticle_t Level_addParticle = (Level_addParticle_t) 0xa449c;

// Gui

typedef void (*Gui_addMessage_t)(unsigned char *gui, std::string const& text);
static Gui_addMessage_t Gui_addMessage = (Gui_addMessage_t) 0x27820;

// ServerSideNetworkHandler

typedef void (*ServerSideNetworkHandler_displayGameMessage_t)(unsigned char *server_side_network_handler, std::string const& message);
static ServerSideNetworkHandler_displayGameMessage_t ServerSideNetworkHandler_displayGameMessage = (ServerSideNetworkHandler_displayGameMessage_t) 0x750c4;

// SimpleChooseLevelScreen

#define SIMPLE_LEVEL_CHOOSE_SCREEN_SIZE 0x68

typedef unsigned char *(*SimpleChooseLevelScreen_t)(unsigned char *simple_choose_level_screen, std::string const& world_name);
static SimpleChooseLevelScreen_t SimpleChooseLevelScreen = (SimpleChooseLevelScreen_t) 0x31404;

// SelectWorldScreen

typedef std::string (*SelectWorldScreen_getUniqueLevelName_t)(unsigned char *screen, std::string const& name);
static SelectWorldScreen_getUniqueLevelName_t SelectWorldScreen_getUniqueLevelName = (SelectWorldScreen_getUniqueLevelName_t) 0x388ec;

// Touch::SelectWorldScreen

static SelectWorldScreen_getUniqueLevelName_t Touch_SelectWorldScreen_getUniqueLevelName = (SelectWorldScreen_getUniqueLevelName_t) 0x3d82c;

#endif

#pragma GCC diagnostic pop