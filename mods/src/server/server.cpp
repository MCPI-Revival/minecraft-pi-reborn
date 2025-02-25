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
#include <mods/fps/fps.h>

// --only-generate: Ony Generate World And Then Exit
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
static auto &get_property_types() {
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
    } types;
    return types;
}

// Get World Name
static std::string get_world_name() {
    const std::string name = get_server_properties().get_string(get_property_types().world_name);
    std::string safe_name = to_cp437(name);
    return safe_name;
}

// Create/Start World
static void start_world(Minecraft *minecraft) {
    // Get World Name
    const std::string world_name = get_world_name();

    // Log
    INFO("Loading World: %s", world_name.c_str());

    // Peaceful Mode
    Options *options = &minecraft->options;
    options->game_difficulty = get_server_properties().get_bool(get_property_types().peaceful_mode) ? 0 : 2;

    // Specify Level Settings
    LevelSettings settings;
    settings.game_type = get_server_properties().get_int(get_property_types().game_mode);
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

// Check If Running In Whitelist Mode
static bool is_whitelist() {
    return get_server_properties().get_bool(get_property_types().enable_whitelist);
}
// Get Path Of Blacklist (Or Whitelist) File
static std::string get_blacklist_file() {
    std::string file = home_get();
    file += '/';
    file += is_whitelist() ? "whitelist" : "blacklist";
    file += ".txt";
    return file;
}

// Get Vector Of Players In Level
static std::vector<Player *> get_players_in_level(Level *level) {
    return level->players;
}
// Get Player's Username
static std::string get_player_username(const Player *player) {
    const std::string *username = &player->username;
    std::string safe_username = from_cp437(*username);
    return safe_username;
}
// Get Level From Minecraft
static Level *get_level(const Minecraft *minecraft) {
    return minecraft->level;
}

// Find Players With Username And Run Callback
typedef void (*player_callback_t)(Minecraft *minecraft, const std::string &username, Player *player);
static void find_players(Minecraft *minecraft, const std::string &target_username, const player_callback_t callback, const bool all_players) {
    Level *level = get_level(minecraft);
    const std::vector<Player *> players = get_players_in_level(level);
    bool found_player = false;
    for (Player *player : players) {
        // Iterate Players
        std::string username = get_player_username(player);
        if (all_players || username == target_username) {
            // Run Callback
            callback(minecraft, username, player);
            found_player = true;
        }
    }
    if (!all_players && !found_player) {
        INFO("Invalid Player: %s", target_username.c_str());
    }
}

// Get RakNet Objects
static RakNet_RakNetGUID get_rak_net_guid(Player *player) {
    return ((ServerPlayer *) player)->guid;
}
static RakNet_SystemAddress get_system_address(RakNet_RakPeer *rak_peer, RakNet_RakNetGUID guid) {
    // Get SystemAddress
    return rak_peer->GetSystemAddressFromGuid(guid);
}
static RakNet_RakPeer *get_rak_peer(const Minecraft *minecraft) {
    return minecraft->rak_net_instance->peer;
}
static char *get_rak_net_guid_ip(RakNet_RakPeer *rak_peer, const RakNet_RakNetGUID &guid) {
    RakNet_SystemAddress address = get_system_address(rak_peer, guid);
    // Get IP
    return address.ToString(false, '|');
}

// Get IP From Player
static char *get_player_ip(const Minecraft *minecraft, Player *player) {
    RakNet_RakPeer *rak_peer = get_rak_peer(minecraft);
    const RakNet_RakNetGUID guid = get_rak_net_guid(player);
    // Return
    return get_rak_net_guid_ip(rak_peer, guid);
}

// Ban Player
static bool is_ip_in_blacklist(const char *ip);
static void ban_callback(Minecraft *minecraft, const std::string &username, Player *player) {
    // Get IP
    char *ip = get_player_ip(minecraft, player);

    // Ban Player
    INFO("Banned: %s (%s)", username.c_str(), ip);
    // Write To File
    std::ofstream blacklist_output(get_blacklist_file(), std::ios_base::app);
    if (blacklist_output) {
        if (blacklist_output.good()) {
            blacklist_output << "# " << username << '\n' << ip << '\n';
        }
        if (blacklist_output.is_open()) {
            blacklist_output.close();
        }
    }
    // Reload
    is_ip_in_blacklist(nullptr);
}

// Kill Player
static void kill_callback(__attribute__((unused)) Minecraft *minecraft, __attribute__((unused)) const std::string &username, Player *player) {
    player->hurt(nullptr, INT32_MAX);
    INFO("Killed: %s", username.c_str());
}

// List Player
static void list_callback(Minecraft *minecraft, const std::string &username, Player *player) {
    INFO(" - %s (%s)", username.c_str(), get_player_ip(minecraft, player));
}

// Get ServerSideNetworkHandler From Minecraft
static ServerSideNetworkHandler *get_server_side_network_handler(const Minecraft *minecraft) {
    return (ServerSideNetworkHandler *) minecraft->network_handler;
}

// Read STDIN Thread
static pthread_t read_stdin_thread_obj;
static volatile bool stdin_line_ready = false;
static std::string stdin_line;
static void *read_stdin_thread(__attribute__((unused)) void *data) {
    // Loop
    char *line = nullptr;
    size_t len = 0;
    while (getline(&line, &len, stdin) != -1) {
        stdin_line = line;
        stdin_line_ready = true;
        // Wait For Line To Be Read
        while (stdin_line_ready) {}
    }
    free(line);
    return nullptr;
}

// Handle Server Stop
static void handle_server_stop(Minecraft *minecraft) {
    if (compat_check_exit_requested()) {
        INFO("Stopping Server");
        // Save And Exit
        Level *level = get_level(minecraft);
        if (level != nullptr) {
            level->saveLevelData();
        }
        minecraft->leaveGame(false);
        // Kill Reader Thread
        pthread_cancel(read_stdin_thread_obj);
        pthread_join(read_stdin_thread_obj, nullptr);
        stdin_line_ready = false;
        // Stop Game
        SDL_Event event;
        event.type = SDL_QUIT;
        media_SDL_PushEvent(&event);
    }
}

// Handle Commands
bool ServerCommand::has_args() const {
    return name[name.length() - 1] == ' ';
}
std::string ServerCommand::get_lhs_help() const {
    std::string out;
    out.append(4, ' ');
    out += name;
    if (has_args()) {
        out += "<Arguments>";
    }
    return out;
}
std::string ServerCommand::get_full_help(const int max_lhs_length) const {
    std::string out = get_lhs_help();
    out.append(max_lhs_length - out.length(), ' ');
    out += " - ";
    out += comment;
    return out;
}
std::vector<ServerCommand> *server_get_commands(Minecraft *minecraft, ServerSideNetworkHandler *server_side_network_handler) {
    std::vector<ServerCommand> *commands = new std::vector<ServerCommand>;
    // Ban Player
    if (!is_whitelist()) {
        commands->push_back({
            .name = "ban ",
            .comment = "IP-Ban All Players With Specified Username",
            .callback = [minecraft](const std::string &cmd) {
                find_players(minecraft, cmd, ban_callback, false);
            }
        });
    }
    // Reload White/Blacklist
    commands->push_back({
        .name = "reload",
        .comment = std::string("Reload The ") + (is_whitelist() ? "Whitelist" : "Blacklist"),
        .callback = [](__attribute__((unused)) const std::string &cmd) {
            INFO("Reloading %s", is_whitelist() ? "Whitelist" : "Blacklist");
            is_ip_in_blacklist(nullptr);
        }
    });
    // Kill Player
    commands->push_back({
        .name = "kill ",
        .comment = "Kill All Players With Specified Username",
        .callback = [minecraft](const std::string &cmd) {
            find_players(minecraft, cmd, kill_callback, false);
        }
    });
    // Post Message
    commands->push_back({
        .name = "say ",
        .comment = "Print Specified Message To Chat",
        .callback = [server_side_network_handler](const std::string &cmd) {
            // Format Message
            const std::string message = "[Server] " + cmd;
            std::string cpp_string = to_cp437(message);
            // Post Message To Chat
            server_side_network_handler->displayGameMessage(cpp_string);
        }
    });
    // List Players
    commands->push_back({
        .name = "list",
        .comment = "List All Players",
        .callback = [minecraft](__attribute__((unused)) const std::string &cmd) {
            INFO("All Players:");
            find_players(minecraft, "", list_callback, true);
        }
    });
    // Ticks-Per-Second
    commands->push_back({
        .name = "tps",
        .comment = "Print TPS",
        .callback = [](__attribute__((unused)) const std::string &cmd) {
            INFO("TPS: %f", tps);
        }
    });
    // Stop
    commands->push_back({
        .name = "stop",
        .comment = "Stop Server",
        .callback = [](__attribute__((unused)) const std::string &cmd) {
            compat_request_exit();
        }
    });
    // Help Page
    commands->push_back({
        .name = "help",
        .comment = "Print This Message",
        .callback = [commands](__attribute__((unused)) const std::string &cmd) {
            INFO("All Commands:");
            std::string::size_type max_lhs_length = 0;
            for (const ServerCommand &command : *commands) {
                const std::string::size_type lhs_length = command.get_lhs_help().length();
                if (lhs_length > max_lhs_length) {
                    max_lhs_length = lhs_length;
                }
            }
            for (const ServerCommand &command : *commands) {
                INFO("%s", command.get_full_help(max_lhs_length).c_str());
            }
        }
    });
    // Return
    return commands;
}
static void handle_commands(Minecraft *minecraft) {
    // Check If Level Is Generated
    if (minecraft->isLevelGenerated() && stdin_line_ready) {
        // Read Line
        std::string data = std::move(stdin_line);
        data.pop_back(); // Remove Newline
        stdin_line_ready = false;
        // Command Ready; Run It
        ServerSideNetworkHandler *server_side_network_handler = get_server_side_network_handler(minecraft);
        if (server_side_network_handler != nullptr) {
            // Generate Command List
            const std::vector<ServerCommand> *commands = server_get_commands(minecraft, server_side_network_handler);
            // Run
            bool success = false;
            for (const ServerCommand &command : *commands) {
                const bool valid = command.has_args() ? data.rfind(command.name, 0) == 0 : data == command.name;
                if (valid) {
                    command.callback(data.substr(command.name.length()));
                    success = true;
                    break;
                }
            }
            if (!success) {
                INFO("Invalid Command: %s", data.c_str());
            }
            // Free
            delete commands;
        }
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

// Check Blacklist/Whitelist
static bool is_ip_in_blacklist(const char *ip) {
    static std::vector<std::string> ips;
    if (ip == nullptr) {
        // Reload
        ips.clear();
        // Check banned-ips.txt
        const std::string blacklist_file_path = get_blacklist_file();
        std::ifstream blacklist_file(blacklist_file_path, std::ios::binary);
        if (blacklist_file) {
            std::string line;
            while (std::getline(blacklist_file, line)) {
                // Check Line
                if (!line.empty() && line[0] != '#') {
                    ips.push_back(line);
                }
            }
            blacklist_file.close();
        } else {
            ERR("Unable To Read Blacklist/Whitelist");
        }
        return false;
    } else {
        // Check List
        for (std::string &x : ips) {
            if (x == ip) {
                return true;
            }
        }
        return false;
    }
}

// Ban Players
static bool RakNet_RakPeer_IsBanned_injection(__attribute__((unused)) RakNet_RakPeer_IsBanned_t original, __attribute__((unused)) RakNet_RakPeer *rakpeer, const char *ip) {
    // Check List
    const bool ret = is_ip_in_blacklist(ip);
    if (is_whitelist()) {
        return !ret;
    } else {
        return ret;
    }
}

// Log IPs
static Player *ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection(ServerSideNetworkHandler *server_side_network_handler, const RakNet_RakNetGUID &guid) {
    // Call Original Method
    Player *player = server_side_network_handler->popPendingPlayer(guid);

    // Check If Player Is Null
    if (player != nullptr) {
        // Get Data
        const std::string *username = &player->username;
        const Minecraft *minecraft = server_side_network_handler->minecraft;
        RakNet_RakPeer *rak_peer = get_rak_peer(minecraft);
        char *ip = get_rak_net_guid_ip(rak_peer, guid);

        // Log
        INFO("%s Has Joined (IP: %s)", username->c_str(), ip);
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
        for (const ServerProperty *property : ServerProperty::get_all()) {
            properties_file_output << "# " << property->comment << '\n';
            properties_file_output << property->key << '=' << property->def << '\n';
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

    // Create Empty Blacklist/Whitelist File
    std::string blacklist_file_path = get_blacklist_file();
    if (access(blacklist_file_path.c_str(), F_OK) != 0) {
        // Write Default
        std::ofstream blacklist_output(blacklist_file_path);
        blacklist_output << "# Blacklist/Whitelist; Each Line Is One IP Address\n";
        blacklist_output.close();
    }
    // Load Blacklist/Whitelist
    is_ip_in_blacklist(nullptr);

    // Prevent Main Player From Loading
    unsigned char player_patch[4] = {0x00, 0x20, 0xa0, 0xe3}; // "mov r2, #0x0"
    patch((void *) 0x1685c, player_patch);
    // Start World On Launch
    misc_run_on_update(Minecraft_update_injection);
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

    // Log IPs
    overwrite_call((void *) 0x75e54, ServerSideNetworkHandler_popPendingPlayer, ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection);

    // Start Reading STDIN
    pthread_create(&read_stdin_thread_obj, nullptr, read_stdin_thread, nullptr);
}

// Init Server
void init_server() {
    server_init();
    set_and_print_env(MCPI_FEATURE_FLAGS_ENV, get_features());
    set_and_print_env(MCPI_RENDER_DISTANCE_ENV, "Tiny");
    set_and_print_env(MCPI_USERNAME_ENV, get_motd().c_str());
}
