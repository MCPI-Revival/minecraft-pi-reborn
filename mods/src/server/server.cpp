#include <string>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <vector>

#include <sys/ioctl.h>
#include <pthread.h>

#include <unistd.h>

#include <SDL/SDL.h>

#include <libreborn/patch.h>
#include <libreborn/env/env.h>
#include <libreborn/util/string.h>
#include <libreborn/util/util.h>

#include <symbols/minecraft.h>

#include <mods/server/server.h>
#include <mods/init/init.h>
#include <mods/compat/compat.h>
#include <mods/misc/misc.h>
#include <mods/game-mode/game-mode.h>

#include "internal.h"

// --only-generate: Only Generate World And Then Exit
static bool only_generate = false;
__attribute__((constructor)) static void _init_only_generate() {
    only_generate = getenv(_MCPI_ONLY_GENERATE_ENV) != nullptr;
}

// Server Properties
ServerProperties &get_server_properties() {
    static ServerProperties properties;
    return properties;
}

// Default Server Properties
static const auto &get_property_types() {
    static struct {
        const ServerProperty message_of_the_day = ServerProperty("motd", "Minecraft Server", "Message Of The Day");
        const ServerProperty show_minecon_badge = ServerProperty("show-minecon-badge", "false", "Show The MineCon Badge Next To MOTD In Server List");
        const ServerProperty game_mode = ServerProperty("game-mode", "0", "Game Mode (0 = Survival, 1 = Creative)");
        const ServerProperty port = ServerProperty("port", std::to_string(DEFAULT_MULTIPLAYER_PORT), "Port");
        const ServerProperty seed = ServerProperty("seed", "", "World Seed (Blank = Random Seed)");
        const ServerProperty force_mob_spawning = ServerProperty("force-mob-spawning", "false", "Force Mob Spawning (false = Disabled, true = Enabled)");
        const ServerProperty peaceful_mode = ServerProperty("peaceful-mode", "false", "Peaceful Mode (false = Disabled, true = Enabled)");
        const ServerProperty world_name = ServerProperty("world-name", "world", "World To Select");
        const ServerProperty max_players = ServerProperty("max-players", "4", "Maximum Player Count");
        const ServerProperty enable_whitelist = ServerProperty("whitelist", "false", "Enable Whitelist");
        const ServerProperty enable_death_messages = ServerProperty("death-messages", "true", "Enable Death Messages");
        const ServerProperty enable_cave_generation = ServerProperty("generate-caves", "true", "Generate Caves");
        const ServerProperty player_data = ServerProperty("track-player-data", "true", "Save/Load Player Data");
    } types;
    return types;
}

// Create/Start World
static int forced_game_mode;
static void start_world(Minecraft *minecraft) {
    // Get World Name
    std::string world_name = get_server_properties().get_string(get_property_types().world_name);
    INFO("Loading World: %s", world_name.c_str());
    world_name = to_cp437(world_name);

    // Peaceful Mode
    Options *options = &minecraft->options;
    options->game_difficulty = get_server_properties().get_bool(get_property_types().peaceful_mode) ? 0 : 2;

    // Specify Level Settings
    LevelSettings settings;
    settings.game_type = forced_game_mode = get_server_properties().get_int(get_property_types().game_mode);
    const std::string seed_str = get_server_properties().get_string(get_property_types().seed);
    const int32_t seed = get_seed_from_string(seed_str);
    settings.seed = seed;

    // Select Level
    minecraft->selectLevel(world_name, world_name, settings);

    // Don't Open Port When Using --only-generate
    if (!only_generate) {
        // Open Port
        const int port = get_server_properties().get_int(get_property_types().port);
        INFO("Listening On: %i", port);
        minecraft->hostMultiplayer(port);
    }

    // Open ProgressScreen
    ProgressScreen *screen = ProgressScreen::allocate();
    screen = screen->constructor();
    minecraft->setScreen((Screen *) screen);
}

// Force Game-Mode To Match Server Settings
static int LevelData_getTagData_CompoundTag_getInt_injection(MCPI_UNUSED CompoundTag *self, const std::string &key) {
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
        // Kill Reader Thread
        stop_reading_commands();
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
static Player *ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection(ServerSideNetworkHandler *server_side_network_handler, const RakNet_RakNetGUID &guid) {
    // Call Original Method
    Player *player = server_side_network_handler->popPendingPlayer(guid);

    // Handle
    if (player != nullptr) {
        // Get Data
        const std::string username = misc_get_player_username_utf(player);
        const Minecraft *minecraft = server_side_network_handler->minecraft;
        RakNet_RakPeer *rak_peer = minecraft->rak_net_instance->peer;
        const std::string ip = get_rak_net_guid_ip(rak_peer, guid);
        // Log IP
        INFO("%s Has Joined (IP: %s)", username.c_str(), ip.c_str());

        // Player Data
        if (get_server_properties().get_bool(get_property_types().player_data)) {
            _load_playerdata((ServerPlayer *) player);
        }
    }

    // Return
    return player;
}

// Get MOTD
static std::string get_motd() {
    std::string motd(get_server_properties().get_string(get_property_types().message_of_the_day));
    return motd;
}

// Get Feature Flags
static bool loaded_features = false;
static const char *get_features() {
    static std::string features;
    if (!loaded_features) {
        loaded_features = true;

        features.clear();
        if (get_server_properties().get_bool(get_property_types().force_mob_spawning)) {
            features += "Force Mob Spawning|";
        }
        if (get_server_properties().get_bool(get_property_types().enable_death_messages)) {
            features += "Implement Death Messages|";
        }
        if (get_server_properties().get_bool(get_property_types().enable_cave_generation)) {
            features += "Generate Caves|";
        }
    }
    return features.c_str();
}

// Get Max Players
static unsigned char get_max_players() {
    int val = get_server_properties().get_int(get_property_types().max_players);
    if (val < 0) {
        val = 0;
    }
    if (val > 255) {
        val = 255;
    }
    return (unsigned char) val;
}

// Real Init Server
static void server_init() {
    // Open Properties File
    std::string file(home_get());
    file.append("/server.properties");
    std::ifstream properties_file(file, std::ios::binary);

    // Check Properties File
    if (!properties_file) {
        // Write Defaults
        std::ofstream properties_file_output(file);
        get_property_types();
        bool is_first = true;
        for (const ServerProperty *property : ServerProperty::get_all()) {
            if (is_first) {
                is_first = false;
            } else {
                properties_file_output << std::endl;
            }
            properties_file_output << "# " << property->comment << std::endl;
            properties_file_output << property->key << '=' << property->def << std::endl;
        }
        properties_file_output.close();
        // Re-Open File
        properties_file = std::ifstream(file, std::ios::binary);
    }

    // Check Properties File
    if (!properties_file) {
        ERR("Unable To Read Server Properties");
    }
    // Load Properties
    get_server_properties().load(properties_file);
    // Close Properties File
    properties_file.close();

    // Setup Blacklist
    blacklist.is_white = get_server_properties().get_bool(get_property_types().enable_whitelist);
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
    if (get_server_properties().get_bool(get_property_types().show_minecon_badge)) {
        unsigned char minecon_badge_patch[4] = {0x04, 0x1a, 0x9f, 0xe5}; // "ldr r1, [0x741f0]"
        patch((void *) 0x737e4, minecon_badge_patch);
    }

    // Handle Newly Joined Players
    overwrite_call((void *) 0x75e54, ServerSideNetworkHandler_popPendingPlayer, ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection);

    // Start Reading STDIN
    start_reading_commands();

    // Player Data
    if (get_server_properties().get_bool(get_property_types().player_data)) {
        _init_server_playerdata();
    }
}

// Init Server
void init_server() {
    server_init();
    set_and_print_env(MCPI_FEATURE_FLAGS_ENV, get_features());
    set_and_print_env(MCPI_RENDER_DISTANCE_ENV, "Tiny");
    set_and_print_env(MCPI_USERNAME_ENV, get_motd().c_str());
}
