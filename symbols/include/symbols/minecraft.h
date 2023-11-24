#pragma once

#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// bool In C
#ifndef __cplusplus
typedef unsigned char bool;
#endif

// Globals

typedef void (*renderCursor_t)(float x, float y, unsigned char *minecraft);
static renderCursor_t renderCursor = (renderCursor_t) 0x480c4;

typedef void (*sleepMs_t)(int32_t x);
static sleepMs_t sleepMs = (sleepMs_t) 0x13cf4;

typedef int32_t (*sdl_key_to_minecraft_key_t)(int32_t sdl_key);
static sdl_key_to_minecraft_key_t sdl_key_to_minecraft_key = (sdl_key_to_minecraft_key_t) 0x1243c;

typedef void (*anGenBuffers_t)(int32_t count, uint32_t *buffers);
static anGenBuffers_t anGenBuffers = (anGenBuffers_t) 0x5f28c;

static char **default_path = (char **) 0xe264; // /.minecraft/
static char **default_username = (char **) 0x18fd4; // StevePi
static char **minecraft_pi_version = (char **) 0x39d94; // v0.1.1 alpha
static char **options_txt_path = (char **) 0x19bc8; // options.txt
static char **options_txt_fopen_mode_when_loading = (char **) 0x19d24; // w
static char ***feedback_vibration_options_txt_name_1 = (char ***) 0x198a0; // feedback_vibration
static char ***feedback_vibration_options_txt_name_2 = (char ***) 0x194bc; // feedback_vibration
static char ***gfx_lowquality_options_txt_name = (char ***) 0x194c4; // gfx_lowquality
static char **classic_create_button_text = (char **) 0x39bec; // Create

static unsigned char **Material_stone = (unsigned char **) 0x180a9c; // Material

static unsigned char *SOUND_STONE = (unsigned char *) 0x181c80; // Tile::SoundType

static unsigned char **Item_flintAndSteel = (unsigned char **) 0x17ba70; // Item
static unsigned char **Item_snowball = (unsigned char **) 0x17bbb0; // Item
static unsigned char **Item_shears = (unsigned char **) 0x17bbf0; // Item
static unsigned char **Item_egg = (unsigned char **) 0x17bbd0; // Item
static unsigned char **Item_dye_powder = (unsigned char **) 0x17bbe0; // Item
static unsigned char **Item_camera = (unsigned char **) 0x17bc14; // Item

static unsigned char **Tile_chest = (unsigned char **) 0x181d60; // Tile
static unsigned char **Tile_water = (unsigned char **) 0x181b3c; // Tile
static unsigned char **Tile_lava = (unsigned char **) 0x181cc8; // Tile
static unsigned char **Tile_calmWater = (unsigned char **) 0x181b40; // Tile
static unsigned char **Tile_calmLava = (unsigned char **) 0x181ccc; // Tile
static unsigned char **Tile_glowingObsidian = (unsigned char **) 0x181dcc; // Tile
static unsigned char **Tile_web = (unsigned char **) 0x181d08; // Tile
static unsigned char **Tile_topSnow = (unsigned char **) 0x181b30; // Tile
static unsigned char **Tile_ice = (unsigned char **) 0x181d80; // Tile
static unsigned char **Tile_invisible_bedrock = (unsigned char **) 0x181d94; // Tile
static unsigned char **Tile_netherReactor = (unsigned char **) 0x181dd0; // Tile
static unsigned char **Tile_info_updateGame1 = (unsigned char **) 0x181c68; // Tile
static unsigned char **Tile_info_updateGame2 = (unsigned char **) 0x181c6c; // Tile
static unsigned char **Tile_bedrock = (unsigned char **) 0x181cc4; // Tile
static unsigned char **Tile_tallgrass = (unsigned char **) 0x181d0c; // Tile
static unsigned char **Tile_stoneSlab = (unsigned char **) 0x181b44; // Tile

static unsigned char **Tile_leaves = (unsigned char **) 0x18120c; // Tile
static unsigned char **Tile_leaves_carried = (unsigned char **) 0x181dd8; // Tile
static unsigned char **Tile_grass = (unsigned char **) 0x181b14; // Tile
static unsigned char **Tile_grass_carried = (unsigned char **) 0x181dd4; // Tile

static float *InvGuiScale = (float *) 0x135d98;

static unsigned char *Options_Option_GRAPHICS = (unsigned char *) 0x136c2c; // Option
static unsigned char *Options_Option_AMBIENT_OCCLUSION = (unsigned char *) 0x136c38; // Option
static unsigned char *Options_Option_ANAGLYPH = (unsigned char *) 0x136c08; // Option

static bool *Minecraft_useAmbientOcclusion = (bool *) 0x136b90;

static bool *HeavyTile_instaFall = (bool *) 0x180cc0;

// Structures

struct AABB {
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
};

struct LevelSettings {
    int32_t seed;
    int32_t game_type;
};

struct RakNet_RakNetGUID {
    unsigned char data[10];
};
struct RakNet_SystemAddress {
    unsigned char data[20];
};

// Tile

static unsigned char **Tile_tiles = (unsigned char **) 0x180e08;

typedef void (*Tile_initTiles_t)();
static Tile_initTiles_t Tile_initTiles = (Tile_initTiles_t) 0xc358c;

#define TILE_SIZE 0x5c
#define TILE_VTABLE_SIZE 0x104

static unsigned char *Tile_vtable = (unsigned char *) 0x115670;

typedef unsigned char *(*Tile_t)(unsigned char *tile, int32_t id, int32_t texture, const void *material);
static Tile_t Tile = (Tile_t) 0xc33a0;

typedef unsigned char *(*Tile_init_t)(unsigned char *tile);
static Tile_init_t Tile_init = (Tile_init_t) 0xc34dc;

typedef unsigned char *(*Tile_setDestroyTime_t)(unsigned char *tile, float destroy_time);
static uint32_t Tile_setDestroyTime_vtable_offset = 0xf8;

typedef unsigned char *(*Tile_setExplodeable_t)(unsigned char *tile, float explodeable);
static uint32_t Tile_setExplodeable_vtable_offset = 0xf4;

typedef unsigned char *(*Tile_setSoundType_t)(unsigned char *tile, unsigned char *sound_type);
static uint32_t Tile_setSoundType_vtable_offset = 0xe8;

typedef int32_t (*Tile_use_t)(unsigned char *tile, unsigned char *level, int32_t x, int32_t y, int32_t z, unsigned char *player);
static uint32_t Tile_use_vtable_offset = 0x98;

typedef int32_t (*Tile_getColor_t)(unsigned char *tile, unsigned char *level_source, int32_t x, int32_t y, int32_t z);
static uint32_t Tile_getColor_vtable_offset = 0xb8;

typedef int32_t (*Tile_getRenderShape_t)(unsigned char *tile);
static uint32_t Tile_getRenderShape_vtable_offset = 0xc;

static uint32_t Tile_id_property_offset = 0x8; // int32_t
static uint32_t Tile_category_property_offset = 0x3c; // int32_t

// GrassTile

static void *GrassTile_getColor_vtable_addr = (void *) 0x111660;

// TallGrass

static Tile_getColor_t TallGrass_getColor = (Tile_getColor_t) 0xc25fc;
static void *TallGrass_getColor_vtable_addr = (void *) 0x1121f0;

// StemTile

static void *StemTile_getColor_vtable_addr = (void *) 0x111088;

// LeafTile

static void *LeafTile_getColor_vtable_addr = (void *) 0x112980;

// LiquidTile

static Tile_getColor_t LiquidTile_getColor = (Tile_getColor_t) 0xc8774;

// TileRenderer

typedef void (*TileRenderer_tesselateBlockInWorld_t)(unsigned char *tile_renderer, unsigned char *tile, int32_t x, int32_t y, int32_t z);
static TileRenderer_tesselateBlockInWorld_t TileRenderer_tesselateBlockInWorld = (TileRenderer_tesselateBlockInWorld_t) 0x59e30;

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

typedef int (*Minecraft_handleBack_t)(unsigned char *minecraft, bool do_nothing);
static uint32_t Minecraft_handleBack_vtable_offset = 0x34;

typedef unsigned char *(*Minecraft_getCreator_t)(unsigned char *minecraft);
static Minecraft_getCreator_t Minecraft_getCreator = (Minecraft_getCreator_t) 0x17538;

typedef unsigned char *(*Minecraft_getLevelSource_t)(unsigned char *minecraft);
static Minecraft_getLevelSource_t Minecraft_getLevelSource = (Minecraft_getLevelSource_t) 0x16e84;

typedef void (*Minecraft_handleMouseDown_t)(unsigned char *minecraft, int param_1, bool can_destroy);
static Minecraft_handleMouseDown_t Minecraft_handleMouseDown = (Minecraft_handleMouseDown_t) 0x1584c;

static uint32_t Minecraft_screen_width_property_offset = 0x20; // int32_t
static uint32_t Minecraft_screen_height_property_offset = 0x24; // int32_t
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
static uint32_t Minecraft_pov_property_offset = 0x150; // Mob *
static uint32_t Minecraft_perf_renderer_property_offset = 0xcbc; // PerfRenderer *
static uint32_t Minecraft_targeted_x_property_offset = 0xc3c; // int32_t
static uint32_t Minecraft_targeted_y_property_offset = 0xc40; // int32_t
static uint32_t Minecraft_targeted_z_property_offset = 0xc44; // int32_t

// GameRenderer

typedef void (*GameRenderer_render_t)(unsigned char *game_renderer, float param_1);
static GameRenderer_render_t GameRenderer_render = (GameRenderer_render_t) 0x4a338;

typedef void (*GameRenderer_setupCamera_t)(unsigned char *game_renderer, float param_1, int param_2);
static GameRenderer_setupCamera_t GameRenderer_setupCamera = (GameRenderer_setupCamera_t) 0x489f8;

static uint32_t GameRenderer_minecraft_property_offset = 0x4; // Minecraft *

// ParticleEngine

typedef void (*ParticleEngine_render_t)(unsigned char *particle_engine, unsigned char *entity, float param_2);
static ParticleEngine_render_t ParticleEngine_render = (ParticleEngine_render_t) 0x43060;

// Mouse

typedef int (*Mouse_get_t)();

static Mouse_get_t Mouse_getX = (Mouse_get_t) 0x1385c;
static Mouse_get_t Mouse_getY = (Mouse_get_t) 0x1386c;

// CommandServer

static uint32_t CommandServer_minecraft_property_offset = 0x18; // Minecraft *

// ServerLevel

#define SERVER_LEVEL_SIZE 0xb80

typedef unsigned char *(*ServerLevel_t)(unsigned char *server_level, unsigned char *storage, unsigned char *name, struct LevelSettings *settings, int param_4, unsigned char *dimension);
static ServerLevel_t ServerLevel = (ServerLevel_t) 0x7692c;

// Packet

typedef void (*Packet_read_t)(unsigned char *packet, unsigned char *bit_stream);

// LoginPacket

static Packet_read_t LoginPacket_read = (Packet_read_t) 0x6e5f8;
static void *LoginPacket_read_vtable_addr = (void *) 0x108dcc;

static uint32_t LoginPacket_username_property_offset = 0xc; // RakString

// StartGamePacket

typedef void (*StartGamePacket_read_t)(unsigned char *packet, unsigned char *bit_stream);
static StartGamePacket_read_t StartGamePacket_read = (StartGamePacket_read_t) 0x72c58;
static void *StartGamePacket_read_vtable_addr = (void *) 0x109264;

static uint32_t StartGamePacket_game_mode_property_offset = 0x14; // int32_t

// ChatPacket

static uint32_t ChatPacket_message_property_offset = 0xc; // char *

// Vec3

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

// HitResult

typedef struct {
    int32_t type;
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t side;
    Vec3 exact;
    unsigned char *entity;
    unsigned char unknown;
} HitResult;

// Options

typedef void (*Options_initDefaultValue_t)(unsigned char *options);
static Options_initDefaultValue_t Options_initDefaultValue = (Options_initDefaultValue_t) 0x18a54;

typedef bool (*Options_getBooleanValue_t)(unsigned char *options, unsigned char *option);
static Options_getBooleanValue_t Options_getBooleanValue = (Options_getBooleanValue_t) 0x1cd74;

static uint32_t Options_options_file_property_offset = 0x10c; // OptionsFile
static uint32_t Options_fancy_graphics_property_offset = 0x17; // unsigned char / bool
static uint32_t Options_split_controls_property_offset = 0x105; // int32_t
static uint32_t Options_game_difficulty_property_offset = 0xe8; // int32_t
static uint32_t Options_3d_anaglyph_property_offset = 0x15; // unsigned char / bool
static uint32_t Options_ambient_occlusion_property_offset = 0x18; // unsigned char / bool
static uint32_t Options_hide_gui_property_offset = 0xec; // unsigned char / bool
static uint32_t Options_third_person_property_offset = 0xed; // unsigned char / bool
static uint32_t Options_render_distance_property_offset = 0x10; // int32_t
static uint32_t Options_sound_property_offset = 0x4; // int32_t
static uint32_t Options_debug_property_offset = 0xee; // unsigned char / bool
static uint32_t Options_server_visible_property_offset = 0x104; // unsigned char / bool
static uint32_t Options_username_property_offset = 0x100; // std::string

// OptionButton

typedef unsigned char *(*OptionButton_t)(unsigned char *option_button, unsigned char *option);
static OptionButton_t OptionButton = (OptionButton_t) 0x1c968;

typedef void (*OptionButton_updateImage_t)(unsigned char *option_button, unsigned char *options);
static OptionButton_updateImage_t OptionButton_updateImage = (OptionButton_updateImage_t) 0x1ca58;

// MouseBuildInput

typedef int32_t (*MouseBuildInput_tickBuild_t)(unsigned char *mouse_build_input, unsigned char *player, uint32_t *build_action_intention_return);
static MouseBuildInput_tickBuild_t MouseBuildInput_tickBuild = (MouseBuildInput_tickBuild_t) 0x17c98;
static void *MouseBuildInput_tickBuild_vtable_addr = (void *) 0x102564;

// ItemInstance

typedef struct {
    int32_t count;
    int32_t id;
    int32_t auxiliary;
} ItemInstance;

typedef ItemInstance *(*ItemInstance_constructor_t)(ItemInstance *item_instance, unsigned char *item);
static ItemInstance_constructor_t ItemInstance_constructor_item = (ItemInstance_constructor_t) 0x9992c;
static ItemInstance_constructor_t ItemInstance_constructor_tile = (ItemInstance_constructor_t) 0x998e4;

typedef ItemInstance *(*ItemInstance_constructor_extra_t)(ItemInstance *item_instance, unsigned char *item, int32_t count, int32_t auxiliary);
static ItemInstance_constructor_extra_t ItemInstance_constructor_tile_extra = (ItemInstance_constructor_extra_t) 0x99918;
static ItemInstance_constructor_extra_t ItemInstance_constructor_item_extra = (ItemInstance_constructor_extra_t) 0x99960;

typedef int32_t (*ItemInstance_getMaxStackSize_t)(ItemInstance *item_instance);
static ItemInstance_getMaxStackSize_t ItemInstance_getMaxStackSize = (ItemInstance_getMaxStackSize_t) 0x99ac8;

// Item

#define ITEM_SIZE 0x24
#define ITEM_VTABLE_SIZE 0x98

static unsigned char *Item_vtable = (unsigned char *) 0x10f128;

typedef void (*Item_initItems_t)();
static Item_initItems_t Item_initItems = (Item_initItems_t) 0x94ed0;

typedef unsigned char *(*Item_t)(unsigned char *item, int32_t id);
static Item_t Item = (Item_t) 0x99488;

typedef void (*Item_setIcon_t)(unsigned char *item, int32_t texture_x, int32_t texture_y);
static uint32_t Item_setIcon_vtable_offset = 0x18;

typedef int32_t (*Item_getIcon_t)(unsigned char *item, int32_t auxiliary);
static uint32_t Item_getIcon_vtable_offset = 0x14;

typedef int32_t (*Item_useOn_t)(unsigned char *item, ItemInstance *item_instance, unsigned char *player, unsigned char *level, int32_t x, int32_t y, int32_t z, int32_t hit_side, float hit_x, float hit_y, float hit_z);
static uint32_t Item_useOn_vtable_offset = 0x20;

static uint32_t Item_id_property_offset = 0x4; // int32_t
static uint32_t Item_is_stacked_by_data_property_offset = 0x19; // unsigned char / bool
static uint32_t Item_category_property_offset = 0x10; // int32_t
static uint32_t Item_max_damage_property_offset = 0x8; // int32_t
static uint32_t Item_max_stack_size_property_offset = 0x14; // int32_t

// TileItem

typedef unsigned char *(*TileItem_t)(unsigned char *tile_item, int32_t id);
static TileItem_t TileItem = (TileItem_t) 0xce3a4;

// AuxDataTileItem

#define AUX_DATA_TILE_ITEM_SIZE 0x2c

static unsigned char *AuxDataTileItem_vtable = (unsigned char *) 0x114a58;

static uint32_t AuxDataTileItem_icon_tile_property_offset = 0x28; // Tile *

// Entity

typedef bool (*Entity_hurt_t)(unsigned char *entity, unsigned char *attacker, int32_t damage);
static uint32_t Entity_hurt_vtable_offset = 0xa4;

static uint32_t Entity_x_property_offset = 0x4; // float
static uint32_t Entity_y_property_offset = 0x8; // float
static uint32_t Entity_z_property_offset = 0xc; // float
static uint32_t Entity_yaw_property_offset = 0x40; // float
static uint32_t Entity_pitch_property_offset = 0x44; // float
static uint32_t Entity_old_x_property_offset = 0x28; // float
static uint32_t Entity_old_y_property_offset = 0x2c; // float
static uint32_t Entity_old_z_property_offset = 0x30; // float
static uint32_t Entity_old_yaw_property_offset = 0x48; // float
static uint32_t Entity_old_pitch_property_offset = 0x4c; // float

// Mob

typedef void (*Mob_actuallyHurt_t)(unsigned char *entity, int32_t damage);
static Mob_actuallyHurt_t Mob_actuallyHurt = (Mob_actuallyHurt_t) 0x7f11c;
static uint32_t Mob_actuallyHurt_vtable_offset = 0x16c;

typedef float (*Mob_getWalkingSpeedModifier_t)(unsigned char *entity);

typedef void (*Mob_die_t)(unsigned char *entity, unsigned char *cause);
static uint32_t Mob_die_vtable_offset = 0x130;

static uint32_t Mob_health_property_offset = 0xec; // int32_t
static uint32_t Mob_texture_property_offset = 0xb54; // std::string

// PathfinderMob

typedef unsigned char *(*PathfinderMob_findAttackTarget_t)(unsigned char *mob);
static uint32_t PathfinderMob_findAttackTarget_vtable_offset = 0x204;

// Player

typedef int (*Player_isUsingItem_t)(unsigned char *player);
static Player_isUsingItem_t Player_isUsingItem = (Player_isUsingItem_t) 0x8f15c;

typedef void (*Player_drop_t)(unsigned char *player, ItemInstance *item_instance, bool is_death);
static uint32_t Player_drop_vtable_offset = 0x208;

static Mob_getWalkingSpeedModifier_t Player_getWalkingSpeedModifier = (Mob_getWalkingSpeedModifier_t) 0x8ea0c;

typedef void (*Player_stopSleepInBed_t)(unsigned char *player, bool param_1, bool param_2, bool param_3);
static uint32_t Player_stopSleepInBed_vtable_offset = 0x228;

static uint32_t Player_username_property_offset = 0xbf4; // std::string
static uint32_t Player_inventory_property_offset = 0xbe0; // Inventory *

// LocalPlayer

static Mob_actuallyHurt_t LocalPlayer_actuallyHurt = (Mob_actuallyHurt_t) 0x44010;
static void *LocalPlayer_actuallyHurt_vtable_addr = (void *) 0x10639c;

static void *LocalPlayer_openTextEdit_vtable_addr = (void *) 0x106460;

static Mob_die_t LocalPlayer_die = (Mob_die_t) 0x45078;
static void *LocalPlayer_die_vtable_addr = (void *) 0x106360;

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

typedef void (*Gui_renderToolBar_t)(unsigned char *gui, float param_1, int32_t param_2, int32_t param_3);
static Gui_renderToolBar_t Gui_renderToolBar = (Gui_renderToolBar_t) 0x26c30;

typedef void (*Gui_renderChatMessages_t)(unsigned char *gui, int32_t y_offset, uint32_t max_messages, bool disable_fading, unsigned char *font);
static Gui_renderChatMessages_t Gui_renderChatMessages = (Gui_renderChatMessages_t) 0x273d8;

static uint32_t Gui_minecraft_property_offset = 0x9f4; // Minecraft *
static uint32_t Gui_selected_item_text_timer_property_offset = 0x9fc; // float

// Textures

typedef void (*Textures_tick_t)(unsigned char *textures, bool param_1);
static Textures_tick_t Textures_tick = (Textures_tick_t) 0x531c4;

// GameMode Constructors

#define CREATOR_MODE_SIZE 0x1c
static void *CreatorMode = (void *) 0x1a044;
#define SURVIVAL_MODE_SIZE 0x24
static void *SurvivalMode = (void *) 0x1b7d8;

// LevelData

typedef uint32_t (*LevelData_getSpawnMobs_t)(unsigned char *level_data);
static LevelData_getSpawnMobs_t LevelData_getSpawnMobs = (LevelData_getSpawnMobs_t) 0xbabec;

// Level

typedef void (*Level_saveLevelData_t)(unsigned char *level);
static Level_saveLevelData_t Level_saveLevelData = (Level_saveLevelData_t) 0xa2e94;

typedef void (*Level_setTileAndData_t)(unsigned char *level, int32_t x, int32_t y, int32_t z, int32_t id, int32_t data);
static Level_setTileAndData_t Level_setTileAndData = (Level_setTileAndData_t) 0xa38b4;

typedef int32_t (*Level_getTile_t)(unsigned char *level, int32_t x, int32_t y, int32_t z);
static Level_getTile_t Level_getTile = (Level_getTile_t) 0xa3380;

typedef unsigned char *(*Level_getMaterial_t)(unsigned char *level, int32_t x, int32_t y, int32_t z);
static Level_getMaterial_t Level_getMaterial = (Level_getMaterial_t) 0xa27f8;

typedef HitResult (*Level_clip_t)(unsigned char *level, unsigned char *param_1, unsigned char *param_2, bool clip_liquids, bool param_3);
static Level_clip_t Level_clip = (Level_clip_t) 0xa3db0;

static uint32_t Level_players_property_offset = 0x60; // std::vector<ServerPlayer *>

// LevelSource

typedef unsigned char *(*LevelSource_getBiome_t)(unsigned char *level_source, int32_t x, int32_t z);
static uint32_t LevelSource_getBiome_vtable_offset = 0x24;

// Material

typedef bool (*Material_isSolid_t)(unsigned char *material);
static uint32_t Material_isSolid_vtable_offset = 0x8;

// LevelRenderer

typedef void (*LevelRenderer_render_t)(unsigned char *level_renderer, unsigned char *mob, int param_1, float delta);
static LevelRenderer_render_t LevelRenderer_render = (LevelRenderer_render_t) 0x4f710;

typedef void (*LevelRenderer_renderDebug_t)(unsigned char *level_renderer, struct AABB *aabb, float delta);
static LevelRenderer_renderDebug_t LevelRenderer_renderDebug = (LevelRenderer_renderDebug_t) 0x4d310;

static uint32_t LevelRenderer_minecraft_property_offset = 0x4; // Minecraft *

// PerfRenderer

typedef void (*PerfRenderer_debugFpsMeterKeyPress_t)(unsigned char *perf_renderer, int key);
static PerfRenderer_debugFpsMeterKeyPress_t PerfRenderer_debugFpsMeterKeyPress = (PerfRenderer_debugFpsMeterKeyPress_t) 0x79118;

// TextEditScreen

#define TEXT_EDIT_SCREEN_SIZE 0xa9

typedef unsigned char *(*TextEditScreen_t)(unsigned char *text_edit_screen, unsigned char *sign);
static TextEditScreen_t TextEditScreen = (TextEditScreen_t) 0x3a840;

static void *TextEditScreen_updateEvents_vtable_addr = (void *) 0x10531c;

// ProgressScreen

#define PROGRESS_SCREEN_SIZE 0x4c

typedef unsigned char *(*ProgressScreen_t)(unsigned char *obj);
static ProgressScreen_t ProgressScreen = (ProgressScreen_t) 0x37044;

// OptionsScreen

static void *OptionsScreen_handleBackEvent_vtable_addr = (void *) 0x10499c;

// FurnaceScreen

typedef int32_t (*FurnaceScreen_handleAddItem_t)(unsigned char *furnace_screen, int32_t slot, ItemInstance const *item);
static FurnaceScreen_handleAddItem_t FurnaceScreen_handleAddItem = (FurnaceScreen_handleAddItem_t) 0x327a0;

static uint32_t FurnaceScreen_tile_entity_property_offset = 0x1d0; // FurnaceTileEntity *

// InBedScreen

static void *InBedScreen_handleBackEvent_vtable_addr = (void *) 0x104614;

// FurnaceTileEntity

typedef ItemInstance *(*FurnaceTileEntity_getItem_t)(unsigned char *furnace_tile_entity, int32_t slot);
static uint32_t FurnaceTileEntity_getItem_vtable_offset = 0x2c;

// GuiComponent

typedef void (*GuiComponent_blit_t)(unsigned char *component, int32_t x_dest, int32_t y_dest, int32_t x_src, int32_t y_src, int32_t width_dest, int32_t height_dest, int32_t width_src, int32_t height_src);
static GuiComponent_blit_t GuiComponent_blit = (GuiComponent_blit_t) 0x282a4;

// Screen

typedef void (*Screen_updateEvents_t)(unsigned char *screen);
static Screen_updateEvents_t Screen_updateEvents = (Screen_updateEvents_t) 0x28eb8;

typedef void (*Screen_keyboardNewChar_t)(unsigned char *screen, char key);
static uint32_t Screen_keyboardNewChar_vtable_offset = 0x70;

typedef void (*Screen_keyPressed_t)(unsigned char *screen, int32_t key);
static uint32_t Screen_keyPressed_vtable_offset = 0x6c;

typedef void (*Screen_init_t)(unsigned char *screen);

typedef void (*Screen_tick_t)(unsigned char *screen);

typedef void (*Screen_render_t)(unsigned char *screen, int32_t param_1, int32_t param_2, float param_3);
static Screen_render_t Screen_render = (Screen_render_t) 0x28a00;

typedef int32_t (*Screen_handleBackEvent_t)(unsigned char *screen, bool param_1);

typedef void (*Screen_buttonClicked_t)(unsigned char *screen, unsigned char *button);

static uint32_t Screen_minecraft_property_offset = 0x14; // Minecraft *
static uint32_t Screen_rendered_buttons_property_offset = 0x18; // std::vector<Button *>
static uint32_t Screen_selectable_buttons_property_offset = 0x30; // std::vector<Button *>
static uint32_t Screen_width_property_offset = 0x8; // int32_t
static uint32_t Screen_height_property_offset = 0xc; // int32_t
static uint32_t Screen_passthrough_input_property_offset = 0x10; // bool

// Button

typedef int32_t (*Button_hovered_t)(unsigned char *button, unsigned char *minecraft, int32_t click_x, int32_t click_y);
static Button_hovered_t Button_hovered = (Button_hovered_t) 0x1be2c;

static uint32_t Button_width_property_offset = 0x14; // int32_t
static uint32_t Button_height_property_offset = 0x18; // int32_t
static uint32_t Button_x_property_offset = 0xc; // int32_t
static uint32_t Button_y_property_offset = 0x10; // int32_t

// StartMenuScreen

static Screen_init_t StartMenuScreen_init = (Screen_init_t) 0x39cc0;
static void *StartMenuScreen_init_vtable_addr = (void *) 0x105194;

static Screen_buttonClicked_t StartMenuScreen_buttonClicked = (Screen_buttonClicked_t) 0x397b0;
static void *StartMenuScreen_buttonClicked_vtable_addr = (void *) 0x1051e8;

static uint32_t StartMenuScreen_options_button_property_offset = 0x98; // Button
static uint32_t StartMenuScreen_create_button_property_offset = 0xc0; // Button

// PauseScreen

static Screen_init_t PauseScreen_init = (Screen_init_t) 0x36810;
static void *PauseScreen_init_vtable_addr = (void *) 0x104b2c;

typedef void (*PauseScreen_updateServerVisibilityText_t)(unsigned char *screen);
static PauseScreen_updateServerVisibilityText_t PauseScreen_updateServerVisibilityText = (PauseScreen_updateServerVisibilityText_t) 0x366b8;

static uint32_t PauseScreen_server_visibility_button_property_offset = 0x60; // Button *

// Touch::IngameBlockSelectionScreen

#define TOUCH_INGAME_BLOCK_SELECTION_SCREEN_SIZE 0x16c

typedef unsigned char *(*Touch_IngameBlockSelectionScreen_t)(unsigned char *screen);
static Touch_IngameBlockSelectionScreen_t Touch_IngameBlockSelectionScreen = (Touch_IngameBlockSelectionScreen_t) 0x3afbc;

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

// FillingContainer

typedef void (*FillingContainer_addItem_t)(unsigned char *filling_container, ItemInstance *item_instance);
static FillingContainer_addItem_t FillingContainer_addItem = (FillingContainer_addItem_t) 0x92aa0;

typedef ItemInstance *(*FillingContainer_getItem_t)(unsigned char *filling_container, int32_t slot);
static uint32_t FillingContainer_getItem_vtable_offset = 0x8;

typedef void (*FillingContainer_setItem_t)(unsigned char *filling_container, int32_t slot, ItemInstance *item_instance);
static uint32_t FillingContainer_setItem_vtable_offset = 0xc;

typedef bool (*FillingContainer_add_t)(unsigned char *filling_container, ItemInstance *item_instance);
static uint32_t FillingContainer_add_vtable_offset = 0x30;

typedef void (*FillingContainer_clearSlot_t)(unsigned char *filling_container, int32_t slot);
static FillingContainer_clearSlot_t FillingContainer_clearSlot = (FillingContainer_clearSlot_t) 0x922f8;

typedef void (*FillingContainer_release_t)(unsigned char *filling_container, int32_t slot);
static FillingContainer_release_t FillingContainer_release = (FillingContainer_release_t) 0x92058;

typedef void (*FillingContainer_compressLinkedSlotList_t)(unsigned char *filling_container, int32_t slot);
static FillingContainer_compressLinkedSlotList_t FillingContainer_compressLinkedSlotList = (FillingContainer_compressLinkedSlotList_t) 0x92280;

static uint32_t FillingContainer_linked_slots_property_offset = 0xc; // int32_t[]
static uint32_t FillingContainer_linked_slots_length_property_offset = 0x14; // int32_t
static uint32_t FillingContainer_is_creative_property_offset = 0x24; // bool

// RakNet::RakString

typedef unsigned char *(*RakNet_RakString_t)(unsigned char *rak_string, const char *format, ...);
static RakNet_RakString_t RakNet_RakString = (RakNet_RakString_t) 0xea5cc;

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

typedef void (*RakNetInstance_pingForHosts_t)(unsigned char *rak_net_instance, int32_t base_port);
static RakNetInstance_pingForHosts_t RakNetInstance_pingForHosts = (RakNetInstance_pingForHosts_t) 0x73538;
static uint32_t RakNetInstance_pingForHosts_vtable_offset = 0x14;
static void *RakNetInstance_pingForHosts_vtable_addr = (void *) 0x109afc;

typedef unsigned char *(*RakNetInstance_t)(unsigned char *rak_net_instance);
static RakNetInstance_t RakNetInstance = (RakNetInstance_t) 0x73b20;

static uint32_t RakNetInstance_peer_property_offset = 0x4; // RakNet::RakPeer *
static uint32_t RakNetInstance_pinging_for_hosts_property_offset = 0x24; // unsigned char

// RakNet::RakPeer

typedef enum {
    RAKNET_STARTED = 0,
    RAKNET_ALREADY_STARTED,
    INVALID_SOCKET_DESCRIPTORS,
    INVALID_MAX_CONNECTIONS,
    SOCKET_FAMILY_NOT_SUPPORTED,
    SOCKET_PORT_ALREADY_IN_USE,
    SOCKET_FAILED_TO_BIND,
    SOCKET_FAILED_TEST_SEND,
    PORT_CANNOT_BE_ZERO,
    FAILED_TO_CREATE_NETWORK_THREAD,
    COULD_NOT_GENERATE_GUID,
    STARTUP_OTHER_FAILURE
} RakNet_StartupResult;
typedef RakNet_StartupResult (*RakNet_RakPeer_Startup_t)(unsigned char *rak_peer, unsigned short maxConnections, unsigned char *socketDescriptors, uint32_t socketDescriptorCount, int32_t threadPriority);
static RakNet_RakPeer_Startup_t RakNet_RakPeer_Startup = (RakNet_RakPeer_Startup_t) 0xe1654;
static void *RakNet_RakPeer_Startup_vtable_addr = (void *) 0x135438;

typedef struct RakNet_SystemAddress (*RakNet_RakPeer_GetSystemAddressFromGuid_t)(unsigned char *rak_peer, struct RakNet_RakNetGUID guid);
static uint32_t RakNet_RakPeer_GetSystemAddressFromGuid_vtable_offset = 0xd0;

typedef bool (*RakNet_RakPeer_IsBanned_t)(unsigned char *rak_peer, const char *ip);
static RakNet_RakPeer_IsBanned_t RakNet_RakPeer_IsBanned = (RakNet_RakPeer_IsBanned_t) 0xda3b4;

typedef bool (*RakNet_RakPeer_Ping_t)(unsigned char *rak_peer, const char *host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections, uint32_t connectionSocketIndex);
static RakNet_RakPeer_Ping_t RakNet_RakPeer_Ping = (RakNet_RakPeer_Ping_t) 0xd9c2c;

// RakNet::SystemAddress

typedef char *(*RakNet_SystemAddress_ToString_t)(struct RakNet_SystemAddress *system_address, bool print_delimiter, char delimiter);
static RakNet_SystemAddress_ToString_t RakNet_SystemAddress_ToString = (RakNet_SystemAddress_ToString_t) 0xd6198;

// ServerSideNetworkHandler

typedef void (*ServerSideNetworkHandler_onDisconnect_t)(unsigned char *server_side_network_handler, struct RakNet_RakNetGUID *guid);
static ServerSideNetworkHandler_onDisconnect_t ServerSideNetworkHandler_onDisconnect = (ServerSideNetworkHandler_onDisconnect_t) 0x75164;
static void *ServerSideNetworkHandler_onDisconnect_vtable_addr = (void *) 0x109bb0;

typedef unsigned char *(*ServerSideNetworkHandler_getPlayer_t)(unsigned char *server_side_network_handler, struct RakNet_RakNetGUID *guid);
static ServerSideNetworkHandler_getPlayer_t ServerSideNetworkHandler_getPlayer = (ServerSideNetworkHandler_getPlayer_t) 0x75464;

typedef unsigned char *(*ServerSideNetworkHandler_popPendingPlayer_t)(unsigned char *server_side_network_handler, struct RakNet_RakNetGUID *guid);
static ServerSideNetworkHandler_popPendingPlayer_t ServerSideNetworkHandler_popPendingPlayer = (ServerSideNetworkHandler_popPendingPlayer_t) 0x75db4;

typedef void (*ServerSideNetworkHandler_handle_t)(unsigned char *server_side_network_handler, struct RakNet_RakNetGUID *rak_net_guid, unsigned char *packet);

static void *ServerSideNetworkHandler_handle_ChatPacket_vtable_addr = (void *) 0x109c60;

static uint32_t ServerSideNetworkHandler_minecraft_property_offset = 0x8; // Minecraft *

// Inventory

typedef void (*Inventory_selectSlot_t)(unsigned char *inventory, int32_t slot);
static Inventory_selectSlot_t Inventory_selectSlot = (Inventory_selectSlot_t) 0x8d13c;

typedef ItemInstance *(*Inventory_getSelected_t)(unsigned char *inventory);
static Inventory_getSelected_t Inventory_getSelected = (Inventory_getSelected_t) 0x8d134;

static uint32_t Inventory_selectedSlot_property_offset = 0x28; // int32_t

// TripodCameraRenderer

#define TRIPOD_CAMERA_RENDERER_SIZE 0x178

typedef unsigned char *(*TripodCameraRenderer_t)(unsigned char *renderer);
static TripodCameraRenderer_t TripodCameraRenderer = (TripodCameraRenderer_t) 0x6583c;

// EntityRenderDispatcher

typedef unsigned char *(*EntityRenderDispatcher_t)(unsigned char *dispatcher);
static EntityRenderDispatcher_t EntityRenderDispatcher = (EntityRenderDispatcher_t) 0x6096c;

typedef void (*EntityRenderDispatcher_assign_t)(unsigned char *dispatcher, unsigned char entity_id, unsigned char *renderer);
static EntityRenderDispatcher_assign_t EntityRenderDispatcher_assign = (EntityRenderDispatcher_assign_t) 0x6094c;

// TileEntity

static uint32_t TileEntity_id_property_offset = 0x18; // int32_t
static uint32_t TileEntity_renderer_id_property_offset = 0x24; // int32_t

// ChestTileEntity

typedef unsigned char *(*ChestTileEntity_t)(unsigned char *tile_entity);
static ChestTileEntity_t ChestTileEntity = (ChestTileEntity_t) 0xcfa78;

// ModelPart

typedef void (*ModelPart_render_t)(unsigned char *model_part, float scale);
static ModelPart_render_t ModelPart_render = (ModelPart_render_t) 0x416dc;

// ItemRenderer

typedef void (*ItemRenderer_renderGuiItem_one_t)(unsigned char *font, unsigned char *textures, ItemInstance *item_instance, float param_1, float param_2, bool param_3);
static ItemRenderer_renderGuiItem_one_t ItemRenderer_renderGuiItem_one = (ItemRenderer_renderGuiItem_one_t) 0x63e58;

typedef void (*ItemRenderer_renderGuiItem_two_t)(unsigned char *font, unsigned char *textures, ItemInstance *item_instance, float param_1, float param_2, float param_3, float param_4, bool param_5);
static ItemRenderer_renderGuiItem_two_t ItemRenderer_renderGuiItem_two = (ItemRenderer_renderGuiItem_two_t) 0x63be0;

typedef void (*ItemRenderer_renderGuiItemCorrect_t)(unsigned char *font, unsigned char *textures, ItemInstance *item_instance, int32_t param_1, int32_t param_2);
static ItemRenderer_renderGuiItemCorrect_t ItemRenderer_renderGuiItemCorrect = (ItemRenderer_renderGuiItemCorrect_t) 0x639a0;

// Tesselator

typedef void (*Tesselator_begin_t)(unsigned char *tesselator, int32_t mode);
static Tesselator_begin_t Tesselator_begin = (Tesselator_begin_t) 0x529d4;

typedef void (*Tesselator_colorABGR_t)(unsigned char *tesselator, int32_t color);
static Tesselator_colorABGR_t Tesselator_colorABGR = (Tesselator_colorABGR_t) 0x52b54;

typedef void (*Tesselator_color_t)(unsigned char *tesselator, int32_t r, int32_t g, int32_t b, int32_t a);
static Tesselator_color_t Tesselator_color = (Tesselator_color_t) 0x52a48;

typedef void (*Tesselator_vertexUV_t)(unsigned char *tesselator, float x, float y, float z, float u, float v);
static Tesselator_vertexUV_t Tesselator_vertexUV = (Tesselator_vertexUV_t) 0x52d40;

// SoundEngine

typedef void (*SoundEngine_init_t)(unsigned char *sound_engine, unsigned char *minecraft, unsigned char *options);
static SoundEngine_init_t SoundEngine_init = (SoundEngine_init_t) 0x67760;

typedef void (*SoundEngine_enable_t)(unsigned char *sound_engine, bool state);
static SoundEngine_enable_t SoundEngine_enable = (SoundEngine_enable_t) 0x6776c;

typedef void (*SoundEngine_update_t)(unsigned char *sound_engine, unsigned char *listener_mob, float listener_angle);
static SoundEngine_update_t SoundEngine_update = (SoundEngine_update_t) 0x67778;

static uint32_t SoundEngine_minecraft_property_offset = 0xa08; // Minecraft *
static uint32_t SoundEngine_options_property_offset = 0x4; // Options *

// Recipes

// If there are multiple item IDs, the priority order is: "item" > "tile" > "instance"
typedef struct {
    unsigned char *item;
    unsigned char *tile;
    ItemInstance instance;
    char letter;
} Recipes_Type;

typedef unsigned char *(*Recipes_t)(unsigned char *recipes);
static Recipes_t Recipes = (Recipes_t) 0x9cabc;

// FurnaceRecipes

static Recipes_t FurnaceRecipes = (Recipes_t) 0xa0778;

// HumanoidMobRenderer

typedef void (*HumanoidMobRenderer_render_t)(unsigned char *model_renderer, unsigned char *entity, float param_2, float param_3, float param_4, float param_5, float param_6);
static HumanoidMobRenderer_render_t HumanoidMobRenderer_render = (HumanoidMobRenderer_render_t) 0x62b8c;

static uint32_t HumanoidMobRenderer_model_property_offset = 0x14; // HumanoidModel *

// HumanoidModel

static uint32_t HumanoidModel_is_sneaking_property_offset = 0x236; // bool

// PlayerRenderer

static void *PlayerRenderer_render_vtable_addr = (void *) 0x107f08;

// WorkbenchScreen

#define WORKBENCH_SCREEN_SIZE 0x180

typedef unsigned char *(*WorkbenchScreen_t)(unsigned char *screen, int32_t param_1);
static WorkbenchScreen_t WorkbenchScreen = (WorkbenchScreen_t) 0x301cc;

// Biome

static uint32_t Biome_color_property_offset = 0x2c; // int32_t
static uint32_t Biome_leaf_color_property_offset = 0x34; // int32_t

// LargeFeature

typedef void (*LargeFeature_apply_t)(unsigned char *large_feature, unsigned char *chunk_source, unsigned char *level, int32_t chunk_x, int32_t chunk_y, unsigned char *chunk_data, int32_t unused);
static uint32_t LargeFeature_apply_vtable_offset = 0x8;

// RandomLevelSource

typedef void (*RandomLevelSource_postProcess_t)(unsigned char *random_level_source, unsigned char *chunk_source, int32_t chunk_x, int32_t chunk_y);
static RandomLevelSource_postProcess_t RandomLevelSource_postProcess = (RandomLevelSource_postProcess_t) 0xb2238;
static void *RandomLevelSource_postProcess_vtable_addr = (void *) 0x1105ac;

typedef void (*RandomLevelSource_buildSurface_t)(unsigned char *random_level_source, int32_t chunk_x, int32_t chunk_y, unsigned char *chunk_data, unsigned char **biomes);
static RandomLevelSource_buildSurface_t RandomLevelSource_buildSurface = (RandomLevelSource_buildSurface_t) 0xb32ec;

static uint32_t RandomLevelSource_cave_feature_property_offset = 0x8; // LargeCaveFeature
static uint32_t RandomLevelSource_level_property_offset = 0x72c8; // Level *

// Method That Require C++ Types
#ifdef __cplusplus

#include <string>
#include <vector>

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

struct Texture {
    int32_t width = 0;
    int32_t height = 0;
    unsigned char *data = NULL;
    int32_t field3_0xc = 0;
    bool field4_0x10 = true;
    bool field5_0x11 = false;
    int32_t field6_0x14 = 0;
    int32_t field7_0x18 = -1;
};

// AppPlatform_linux

typedef Texture (*AppPlatform_linux_loadTexture_t)(unsigned char *app_platform, std::string const& path, bool b);
static AppPlatform_linux_loadTexture_t AppPlatform_linux_loadTexture = (AppPlatform_linux_loadTexture_t) 0x12c20;

// Tile

typedef unsigned char *(*Tile_setDescriptionId_t)(unsigned char *tile, std::string const& description_id);
static uint32_t Tile_setDescriptionId_vtable_offset = 0xe0;

// Item

typedef void (*Item_setDescriptionId_t)(unsigned char *item, std::string const& name);
static uint32_t Item_setDescriptionId_vtable_offset = 0x6c;

typedef std::string (*Item_getDescriptionId_t)(unsigned char *item, const ItemInstance *item_instance);
static uint32_t Item_getDescriptionId_vtable_offset = 0x7c;

// AppPlatform

typedef void (*AppPlatform_saveScreenshot_t)(unsigned char *app_platform, std::string const& path, int32_t width, int32_t height);
static void *AppPlatform_linux_saveScreenshot_vtable_addr = (void *) 0x102160;

typedef AppPlatform_readAssetFile_return_value (*AppPlatform_readAssetFile_t)(unsigned char *app_platform, std::string const& path);
static AppPlatform_readAssetFile_t AppPlatform_readAssetFile = (AppPlatform_readAssetFile_t) 0x12b10;

// Minecraft

typedef void (*Minecraft_selectLevel_t)(unsigned char *minecraft, std::string const& level_dir, std::string const& level_name, LevelSettings const& settings);
static Minecraft_selectLevel_t Minecraft_selectLevel = (Minecraft_selectLevel_t) 0x16f38;

// ExternalFileLevelStorageSource

typedef void (*ExternalFileLevelStorageSource_deleteLevel_t)(unsigned char *external_file_level_storage_source, std::string const& level_name);
static uint32_t ExternalFileLevelStorageSource_deleteLevel_vtable_offset = 0x20;

// CommandServer

typedef std::string (*CommandServer_parse_t)(unsigned char *command_server, struct ConnectedClient &client, std::string const& command);
static CommandServer_parse_t CommandServer_parse = (CommandServer_parse_t) 0x6aa8c;

// Level

typedef void (*Level_addParticle_t)(unsigned char *level, std::string const& particle, float x, float y, float z, float deltaX, float deltaY, float deltaZ, int count);
static Level_addParticle_t Level_addParticle = (Level_addParticle_t) 0xa449c;

// Gui

typedef void (*Gui_addMessage_t)(unsigned char *gui, std::string const& text);
static Gui_addMessage_t Gui_addMessage = (Gui_addMessage_t) 0x27820;

// GuiComponent

typedef void (*GuiComponent_drawCenteredString_t)(unsigned char *component, unsigned char *font, std::string const& text, int32_t x, int32_t y, int32_t color);
static GuiComponent_drawCenteredString_t GuiComponent_drawCenteredString = (GuiComponent_drawCenteredString_t) 0x2821c;

// Button

typedef unsigned char *(*Button_t)(unsigned char *button, int32_t param_1, std::string const& text);
static Button_t Button = (Button_t) 0x1bc54;

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

// SoundEngine

typedef void (*SoundEngine_playUI_t)(unsigned char *sound_engine, std::string const& name, float volume, float pitch);
static SoundEngine_playUI_t SoundEngine_playUI = (SoundEngine_playUI_t) 0x67864;

typedef void (*SoundEngine_play_t)(unsigned char *sound_engine, std::string const& name, float x, float y, float z, float volume, float pitch);
static SoundEngine_play_t SoundEngine_play = (SoundEngine_play_t) 0x67860;

// Common

typedef std::string (*Common_getGameVersionString_t)(std::string const& version_suffix);
static Common_getGameVersionString_t Common_getGameVersionString = (Common_getGameVersionString_t) 0x15068;

// Options

typedef void (*Options_addOptionToSaveOutput_t)(unsigned char *options, std::vector<std::string> &data, std::string option, int32_t value);
static Options_addOptionToSaveOutput_t Options_addOptionToSaveOutput = (Options_addOptionToSaveOutput_t) 0x195e4;

// OptionsFile

typedef std::vector<std::string> (*OptionsFile_getOptionStrings_t)(unsigned char *options_file);
static OptionsFile_getOptionStrings_t OptionsFile_getOptionStrings = (OptionsFile_getOptionStrings_t) 0x19c1c;

typedef void (*OptionsFile_save_t)(unsigned char *options_file, std::vector<std::string> const& data);
static OptionsFile_save_t OptionsFile_save = (OptionsFile_save_t) 0x19bcc;

static uint32_t OptionsFile_options_txt_path_property_offset = 0x0; // std::string

// OptionsPane

typedef void (*OptionsPane_unknown_toggle_creating_function_t)(unsigned char *options_pane, uint32_t group_id, std::string const& name, unsigned char *option);
static OptionsPane_unknown_toggle_creating_function_t OptionsPane_unknown_toggle_creating_function = (OptionsPane_unknown_toggle_creating_function_t) 0x24470;

// Textures

typedef int32_t (*Textures_loadAndBindTexture_t)(unsigned char *textures, std::string const& name);
static Textures_loadAndBindTexture_t Textures_loadAndBindTexture = (Textures_loadAndBindTexture_t) 0x539cc;

typedef int32_t (*Textures_assignTexture_t)(unsigned char *textures, std::string const& name, unsigned char *data);
static Textures_assignTexture_t Textures_assignTexture = (Textures_assignTexture_t) 0x5354c;

// Recipes

typedef void (*Recipes_addShapelessRecipe_t)(unsigned char *recipes, ItemInstance const& result, std::vector<Recipes_Type> const& ingredients);
static Recipes_addShapelessRecipe_t Recipes_addShapelessRecipe = (Recipes_addShapelessRecipe_t) 0x9c3dc;

typedef void (*Recipes_addShapedRecipe_1_t)(unsigned char *recipes, ItemInstance const& result, std::string const& line_1, std::vector<Recipes_Type> const& ingredients);
static Recipes_addShapedRecipe_1_t Recipes_addShapedRecipe_1 = (Recipes_addShapedRecipe_1_t) 0x9ca74;

typedef void (*Recipes_addShapedRecipe_2_t)(unsigned char *recipes, ItemInstance const& result, std::string const& line_1, std::string const& line_2, std::vector<Recipes_Type> const& ingredients);
static Recipes_addShapedRecipe_2_t Recipes_addShapedRecipe_2 = (Recipes_addShapedRecipe_2_t) 0x9ca24;

typedef void (*Recipes_addShapedRecipe_3_t)(unsigned char *recipes, ItemInstance const& result, std::string const& line_1, std::string const& line_2, std::string const& line_3, std::vector<Recipes_Type> const& ingredients);
static Recipes_addShapedRecipe_3_t Recipes_addShapedRecipe_3 = (Recipes_addShapedRecipe_3_t) 0x9c9d0;

// FurnaceRecipes

typedef void (*FurnaceRecipes_addFurnaceRecipe_t)(unsigned char *recipes, int32_t input_item_id, ItemInstance const& result);
static FurnaceRecipes_addFurnaceRecipe_t FurnaceRecipes_addFurnaceRecipe = (FurnaceRecipes_addFurnaceRecipe_t) 0xa0714;

// FurnaceTileEntity

typedef int32_t (*FurnaceTileEntity_getBurnDuration_t)(ItemInstance const& item_instance);
static FurnaceTileEntity_getBurnDuration_t FurnaceTileEntity_getBurnDuration = (FurnaceTileEntity_getBurnDuration_t) 0xd33f8;

#endif

#pragma GCC diagnostic pop
