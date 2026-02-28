#include <fstream>
#include <limits>

#include <SDL/SDL.h>

#include <libreborn/patch.h>
#include <libreborn/env/env.h>
#include <libreborn/util/string.h>
#include <libreborn/util/util.h>

#include <mods/server/server.h>
#include <mods/init/init.h>
#include <mods/compat/compat.h>
#include <mods/misc/misc.h>
#include <mods/game-mode/game-mode.h>
#include <mods/feature/feature.h>
#include <mods/options/options.h>

#include "internal.h"

// --only-generate: Only Generate World And Then Exit
static bool only_generate = false;
static void init_only_generate() {
    only_generate = is_env_set(_MCPI_ONLY_GENERATE_ENV);
}

// Server Properties
struct RebornServerProperties : ServerProperties {
    const ServerProperty<std::string> message_of_the_day = ServerProperty<std::string>(*this, "motd", "Minecraft Server", "Message Of The Day");
    const ServerProperty<bool> show_minecon_badge = ServerProperty(*this, "show-minecon-badge", false, "Show The MineCon Badge Next To MOTD In Server List");
    const ServerProperty<int> game_mode = ServerProperty(*this, "game-mode", 0, "Game Mode (0 = Survival, 1 = Creative)");
    const ServerProperty<int> port = ServerProperty(*this, "port", DEFAULT_MULTIPLAYER_PORT, "Port");
    const ServerProperty<std::string> seed = ServerProperty<std::string>(*this, "seed", "", "World Seed (Blank = Random Seed)");
    const ServerProperty<bool> force_mob_spawning = ServerProperty(*this, "force-mob-spawning", false, "Force Mob Spawning");
    const ServerProperty<bool> force_daynight_cycle = ServerProperty(*this, "force-daynight-cycle", false, "Force Enable The Day/Night Cycle");
    const ServerProperty<bool> peaceful_mode = ServerProperty(*this, "peaceful-mode", false, "Peaceful Mode");
    const ServerProperty<std::string> world_name = ServerProperty<std::string>(*this, "world-name", "world", "World To Select");
    const ServerProperty<int> max_players = ServerProperty(*this, "max-players", 4, "Maximum Player Count");
    const ServerProperty<bool> enable_whitelist = ServerProperty(*this, "whitelist", false, "Enable Whitelist");
    const ServerProperty<bool> death_messages = ServerProperty(*this, "death-messages", true, "Enable Death Messages");
    const ServerProperty<bool> generate_caves = ServerProperty(*this, "generate-caves", true, "Generate Caves");
    const ServerProperty<bool> generate_tall_grass = ServerProperty(*this, "generate-tall-grass", false, "Generate Tall Grass");
    const ServerProperty<bool> player_data = ServerProperty(*this, "track-player-data", false, "Save/Load Player Data");
    const ServerProperty<bool> is_vanilla_compatible = ServerProperty(*this, "is-vanilla-compatible", false, "Whether The Server Is Compatible With Vanilla Clients");
    // Singleton
    static RebornServerProperties &get() {
        static RebornServerProperties properties;
        return properties;
    }
};
ServerProperties &get_server_properties() {
    return RebornServerProperties::get();
}

// Create/Start World
static int forced_game_mode;
static void start_world(Minecraft *minecraft) {
    // Get World Name
    std::string world_name = RebornServerProperties::get().world_name.get();
    INFO("Loading World: %s", world_name.c_str());
    world_name = to_cp437(world_name);

    // Peaceful Mode
    Options *options = &minecraft->options;
    options->game_difficulty = RebornServerProperties::get().peaceful_mode.get() ? difficulty_peaceful : difficulty_normal;

    // Specify Level Settings
    LevelSettings settings = {};
    settings.game_type = forced_game_mode = RebornServerProperties::get().game_mode.get();
    const std::string seed_str = RebornServerProperties::get().seed.get();
    const int32_t seed = get_seed_from_string(seed_str);
    settings.seed = seed;

    // Select Level
    minecraft->selectLevel(world_name, world_name, settings);

    // Don't Open Port When Using --only-generate
    if (!only_generate) {
        // Open Port
        const int port = RebornServerProperties::get().port.get();
        INFO("Listening On: %i", port);
        minecraft->hostMultiplayer(port);
    }

    // Open ProgressScreen
    ProgressScreen *screen = ProgressScreen::allocate();
    screen = screen->constructor();
    minecraft->setScreen((Screen *) screen);
}

// Force Game-Mode To Match Server Settings
static int LevelData_getTagData_CompoundTag_getInt_injection(MCPI_UNUSED const CompoundTag *self, const std::string &key) {
    if (key != "GameType") {
        IMPOSSIBLE();
    }
    return forced_game_mode;
}

// Handle Server Stop
static void handle_server_stop(Minecraft *minecraft) {
    if (compat_check_exit_requested()) {
        INFO("Stopping Server");
        // Save And Exit
        Level *level = minecraft->level;
        if (level != nullptr && minecraft->isLevelGenerated()) {
            level->saveGame();
            ChunkSource *chunk_source = level->chunk_source;
            if (chunk_source) {
                chunk_source->saveAll(true);
            }
        }
        minecraft->leaveGame(false);
        // Stop Game
        SDL_Event event;
        event.type = SDL_QUIT;
        media_SDL_PushEvent(&event);
    }
}

// Runs Every Tick
static bool loaded = false;
static void Minecraft_update_injection(Minecraft *minecraft) {
    // Create/Start World
    if (!loaded) {
        start_world(minecraft);
        loaded = true;
    }

    // Handle --only-generate
    if (only_generate && minecraft->isLevelGenerated()) {
        // Request Exit
        compat_request_exit();
        // Disable Special Behavior After Requesting Exit
        only_generate = false;
    }

    // Handle Commands
    handle_commands(minecraft);

    // Server Stop
    handle_server_stop(minecraft);
}

// Ban Players
static bool RakNet_RakPeer_IsBanned_injection(MCPI_UNUSED RakNet_RakPeer_IsBanned_t original, MCPI_UNUSED RakNet_RakPeer *rakpeer, const char *ip) {
    // Check List
    bool ret = blacklist.contains(ip);
    if (blacklist.is_white) {
        ret = !ret;
    }
    return ret;
}

// Handle New Player
static bool should_save_player_data() {
    return RebornServerProperties::get().player_data.get();
}
static Player *ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection(ServerSideNetworkHandler *server_side_network_handler, const RakNet_RakNetGUID &guid) {
    // Call Original Method
    Player *player = server_side_network_handler->popPendingPlayer(guid);

    // Handle
    if (player != nullptr) {
        // Get Data
        const std::string username = misc_get_player_username_utf(player);
        const Minecraft *minecraft = server_side_network_handler->minecraft;
        const RakNet_RakPeer *rak_peer = minecraft->rak_net_instance->peer;
        const std::string ip = get_rak_net_guid_ip(rak_peer, guid);
        // Log IP
        INFO("%s Has Joined (IP: %s)", username.c_str(), ip.c_str());

        // Player Data
        if (should_save_player_data()) {
            _load_playerdata((ServerPlayer *) player);
        }
    }

    // Return
    return player;
}

// Crash On Failed Level Loads
static volatile bool load_success;
static volatile bool file_exists;
static void ExternalFileLevelStorage_readLevelData_LevelData_v1_read_injection(LevelData *self, RakNet_BitStream &bit_stream, const int param_1) {
    load_success = true;
    self->v1_read(bit_stream, param_1);
}
static void ExternalFileLevelStorage_readLevelData_LevelData_getTagData_injection(LevelData *self, const CompoundTag *tag) {
    load_success = true;
    self->getTagData(tag);
}
static size_t ExternalFileLevelStorage_readLevelData_fread_injection(void *ptr, const size_t size, const size_t n, FILE *stream) {
    file_exists = true;
    return fread(ptr, size, n, stream);
}
static bool ExternalFileLevelStorage_readLevelData_injection(ExternalFileLevelStorage_readLevelData_t original, const std::string &directory, LevelData &data) {
    load_success = false;
    file_exists = false;
    const bool ret = original(directory, data);
    const bool load_fail = !ret || !load_success;
    if (file_exists && load_fail) {
        ERR("Corrupted Level Detected!");
    }
    return ret;
}

// Get MOTD
static std::string get_motd() {
    return RebornServerProperties::get().message_of_the_day.get();
}

// Get Max Players
static unsigned char get_max_players() {
    int val = RebornServerProperties::get().max_players.get();
    typedef std::numeric_limits<unsigned char> limits;
    val = std::max<int>(val, limits::min());
    val = std::min<int>(val, limits::max());
    return (unsigned char) val;
}

// Load Properties
void init_server_flags() {
    // Open Properties File
    std::string file = home_get() + "/server.properties";
    std::ifstream properties_file(file, std::ios::in);

    // Check Properties File
    if (!properties_file) {
        // Write Defaults
        std::ofstream properties_file_output(file, std::ios::out);
        RebornServerProperties::get();
        bool is_first = true;
        for (const ServerPropertyBase *property : ServerPropertyBase::get_all()) {
            if (is_first) {
                is_first = false;
            } else {
                properties_file_output << '\n';
            }
            properties_file_output << property->to_string();
        }
        properties_file_output.close();
        // Re-Open File
        properties_file = std::ifstream(file, std::ios::in);
    }

    // Check Properties File
    if (!properties_file) {
        ERR("Unable To Read Server Properties");
    }
    // Load Properties
    get_server_properties().load(properties_file);
    // Close Properties File
    properties_file.close();

    // Apply Flags
#define FLAG(name) server_##name = RebornServerProperties::get().name.get()
#include <mods/feature/server.h>
#undef FLAG
    feature_server_flags_set = true;
    set_and_print_env(MCPI_RENDER_DISTANCE_ENV, "Tiny");
    set_and_print_env(MCPI_USERNAME_ENV, get_motd().c_str());
}

// Init Server
void init_server() {
    // Setup Blacklist
    blacklist.is_white = RebornServerProperties::get().enable_whitelist.get();
    blacklist.load();

    // Prevent The Main Player From Loading
    unsigned char player_patch[4] = {0x00, 0x20, 0xa0, 0xe3}; // "mov r2, #0x0"
    patch((void *) 0x1685c, player_patch);
    // Start World On Launch
    misc_run_on_update(Minecraft_update_injection);
    // Force Game-Mode To Match Server Settings
    overwrite_call((void *) 0xbac50, CompoundTag_getInt, LevelData_getTagData_CompoundTag_getInt_injection);
    // Set Max Players
    unsigned char max_players_patch[4] = {get_max_players(), 0x30, 0xa0, 0xe3}; // "mov r3, #MAX_PLAYERS"
    patch((void *) 0x166d0, max_players_patch);
    // Custom Banned IP List
    overwrite_calls(RakNet_RakPeer_IsBanned, RakNet_RakPeer_IsBanned_injection);

    // Show The MineCon Icon Next To MOTD In Server List
    if (RebornServerProperties::get().show_minecon_badge.get()) {
        unsigned char minecon_badge_patch[4] = {0x04, 0x1a, 0x9f, 0xe5}; // "ldr r1, [0x741f0]"
        patch((void *) 0x737e4, minecon_badge_patch);
    }

    // Handle Newly Joined Players
    overwrite_call((void *) 0x75e54, ServerSideNetworkHandler_popPendingPlayer, ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection);

    // Detect Corrupt Levels
    overwrite_calls(ExternalFileLevelStorage_readLevelData, ExternalFileLevelStorage_readLevelData_injection);
    overwrite_call((void *) 0xb8d14, LevelData_v1_read, ExternalFileLevelStorage_readLevelData_LevelData_v1_read_injection);
    overwrite_call((void *) 0xb8d58, LevelData_getTagData, ExternalFileLevelStorage_readLevelData_LevelData_getTagData_injection);
    overwrite_call_manual((void *) 0xb8c7c, (void *) ExternalFileLevelStorage_readLevelData_fread_injection);

    // Player Data
    if (should_save_player_data()) {
        _init_server_playerdata();
    }

    // Setup Variables
    init_only_generate();
}