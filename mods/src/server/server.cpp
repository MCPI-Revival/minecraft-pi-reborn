#include <string>
#include <cstdint>
#include <ctime>
#include <cstdio>
#include <fstream>
#include <vector>

#include <sys/ioctl.h>
#include <pthread.h>

#include <unistd.h>

#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/server/server.h>
#include <mods/init/init.h>
#include <mods/home/home.h>
#include <mods/compat/compat.h>
#include <mods/misc/misc.h>
#include <mods/game-mode/game-mode.h>

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
#define DEFAULT_MOTD "Minecraft Server"
#define DEFAULT_SHOW_MINECON_BADGE "false"
#define DEFAULT_GAME_MODE "0"
#define DEFAULT_PORT "19132"
#define DEFAULT_SEED ""
#define DEFAULT_FORCE_MOB_SPAWNING "false"
#define DEFAULT_PEACEFUL_MODE "false"
#define DEFAULT_WORLD_NAME "world"
#define DEFAULT_MAX_PLAYERS "4"
#define DEFAULT_WHITELIST "false"
#define DEFAULT_DEATH_MESSAGES "true"
#define DEFAULT_GENERATE_CAVES "true"

// Get World Name
static std::string get_world_name() {
    std::string name = get_server_properties().get_string("world-name", DEFAULT_WORLD_NAME);
    char *safe_name_c = to_cp437(name.c_str());
    std::string safe_name = safe_name_c;
    free(safe_name_c);
    return safe_name;
}

// Create/Start World
static void start_world(Minecraft *minecraft) {
    // Get World Name
    std::string world_name = get_world_name();

    // Log
    INFO("Loading World: %s", world_name.c_str());

    // Peaceful Mode
    Options *options = &minecraft->options;
    options->game_difficulty = get_server_properties().get_bool("peaceful-mode", DEFAULT_PEACEFUL_MODE) ? 0 : 2;

    // Specify Level Settings
    LevelSettings settings;
    settings.game_type = get_server_properties().get_int("game-mode", DEFAULT_GAME_MODE);
    const std::string seed_str = get_server_properties().get_string("seed", DEFAULT_SEED);
    const int32_t seed = get_seed_from_string(seed_str);
    settings.seed = seed;

    // Select Level
    minecraft->selectLevel(&world_name, &world_name, &settings);

    // Don't Open Port When Using --only-generate
    if (!only_generate) {
        // Open Port
        const int port = get_server_properties().get_int("port", DEFAULT_PORT);
        INFO("Listening On: %i", port);
        minecraft->hostMultiplayer(port);
    }

    // Open ProgressScreen
    ProgressScreen *screen = new ProgressScreen;
    ALLOC_CHECK(screen);
    screen = screen->constructor();
    minecraft->setScreen((Screen *) screen);
}

// Check If Running In Whitelist Mode
static bool is_whitelist() {
    return get_server_properties().get_bool("whitelist", DEFAULT_WHITELIST);
}
// Get Path Of Blacklist (Or Whitelist) File
static std::string get_blacklist_file() {
    std::string file(home_get());
    file.append(is_whitelist() ? "/whitelist.txt" : "/blacklist.txt");
    return file;
}

// Get Vector Of Players In Level
static std::vector<Player *> get_players_in_level(Level *level) {
    return level->players;
}
// Get Player's Username
static std::string get_player_username(Player *player) {
    std::string *username = &player->username;
    char *safe_username_c = from_cp437(username->c_str());
    std::string safe_username = safe_username_c;
    free(safe_username_c);
    return safe_username;
}
// Get Level From Minecraft
static Level *get_level(Minecraft *minecraft) {
    return minecraft->level;
}

// Find Players With Username And Run Callback
typedef void (*player_callback_t)(Minecraft *minecraft, const std::string &username, Player *player);
static void find_players(Minecraft *minecraft, const std::string &target_username, player_callback_t callback, bool all_players) {
    Level *level = get_level(minecraft);
    std::vector<Player *> players = get_players_in_level(level);
    bool found_player = false;
    for (std::size_t i = 0; i < players.size(); i++) {
        // Iterate Players
        Player *player = players[i];
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
static RakNet_RakPeer *get_rak_peer(Minecraft *minecraft) {
    return minecraft->rak_net_instance->peer;
}
static char *get_rak_net_guid_ip(RakNet_RakPeer *rak_peer, RakNet_RakNetGUID guid) {
    RakNet_SystemAddress address = get_system_address(rak_peer, guid);
    // Get IP
    return address.ToString(false, '|');
}

// Get IP From Player
static char *get_player_ip(Minecraft *minecraft, Player *player) {
    RakNet_RakPeer *rak_peer = get_rak_peer(minecraft);
    RakNet_RakNetGUID guid = get_rak_net_guid(player);
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

// Track TPS
#define NANOSECONDS_IN_SECOND 1000000000ll
static long long int get_time() {
    timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long long int a = (long long int) ts.tv_nsec;
    long long int b = ((long long int) ts.tv_sec) * NANOSECONDS_IN_SECOND;
    return a + b;
}
static bool is_last_tick_time_set = false;
static long long int last_tick_time;
static double tps = 0;
static void Minecraft_tick_injection(__attribute__((unused)) Minecraft *minecraft) {
    long long int time = get_time();
    if (is_last_tick_time_set) {
        long long int tick_time = time - last_tick_time;
        tps = ((double) NANOSECONDS_IN_SECOND) / ((double) tick_time);
    } else {
        is_last_tick_time_set = true;
    }
    last_tick_time = time;
}

// Get ServerSideNetworkHandler From Minecraft
static ServerSideNetworkHandler *get_server_side_network_handler(Minecraft *minecraft) {
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
        SDL_PushEvent(&event);
    }
}

// Handle Commands
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
            static std::string ban_command("ban ");
            static std::string say_command("say ");
            static std::string kill_command("kill ");
            static std::string list_command("list");
            static std::string reload_command("reload");
            static std::string tps_command("tps");
            static std::string stop_command("stop");
            static std::string help_command("help");
            if (!is_whitelist() && data.rfind(ban_command, 0) == 0) {
                // IP-Ban Target Username
                std::string ban_username = data.substr(ban_command.length());
                find_players(minecraft, ban_username, ban_callback, false);
            } else if (data == reload_command) {
                INFO("Reloading %s", is_whitelist() ? "Whitelist" : "Blacklist");
                is_ip_in_blacklist(nullptr);
            } else if (data.rfind(kill_command, 0) == 0) {
                // Kill Target Username
                std::string kill_username = data.substr(kill_command.length());
                find_players(minecraft, kill_username, kill_callback, false);
            } else if (data.rfind(say_command, 0) == 0) {
                // Format Message
                std::string message = "[Server] " + data.substr(say_command.length());
                char *safe_message = to_cp437(message.c_str());
                std::string cpp_string = safe_message;
                // Post Message To Chat
                server_side_network_handler->displayGameMessage(&cpp_string);
                // Free
                free(safe_message);
            } else if (data == list_command) {
                // List Players
                INFO("All Players:");
                find_players(minecraft, "", list_callback, true);
            } else if (data == tps_command) {
                // Print TPS
                INFO("TPS: %f", tps);
            } else if (data == stop_command) {
                // Stop Server
                compat_request_exit();
            } else if (data == help_command) {
                INFO("All Commands:");
                if (!is_whitelist()) {
                    INFO("    ban <Username>  - IP-Ban All Players With Specifed Username");
                }
                INFO("    reload          - Reload The %s", is_whitelist() ? "Whitelist" : "Blacklist");
                INFO("    kill <Username> - Kill All Players With Specifed Username");
                INFO("    say <Message>   - Print Specified Message To Chat");
                INFO("    list            - List All Players");
                INFO("    tps             - Print TPS");
                INFO("    stop            - Stop Server");
                INFO("    help            - Print This Message");
            } else {
                INFO("Invalid Command: %s", data.c_str());
            }
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
        std::string blacklist_file_path = get_blacklist_file();
        std::ifstream blacklist_file(blacklist_file_path);
        if (blacklist_file) {
            if (blacklist_file.good()) {
                std::string line;
                while (std::getline(blacklist_file, line)) {
                    // Check Line
                    if (line.length() > 0 && line[0] != '#') {
                        ips.push_back(line);
                    }
                }
            }
            if (blacklist_file.is_open()) {
                blacklist_file.close();
            }
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
    bool ret = is_ip_in_blacklist(ip);
    if (is_whitelist()) {
        return !ret;
    } else {
        return ret;
    }
}

// Log IPs
static Player *ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection(ServerSideNetworkHandler *server_side_network_handler, RakNet_RakNetGUID *guid) {
    // Call Original Method
    Player *player = server_side_network_handler->popPendingPlayer(guid);

    // Check If Player Is Null
    if (player != nullptr) {
        // Get Data
        std::string *username = &player->username;
        Minecraft *minecraft = server_side_network_handler->minecraft;
        RakNet_RakPeer *rak_peer = get_rak_peer(minecraft);
        char *ip = get_rak_net_guid_ip(rak_peer, *guid);

        // Log
        INFO("%s Has Joined (IP: %s)", username->c_str(), ip);
    }

    // Return
    return player;
}

// Get MOTD
static std::string get_motd() {
    std::string motd(get_server_properties().get_string("motd", DEFAULT_MOTD));
    return motd;
}

// Get Feature Flags
static bool loaded_features = false;
static const char *get_features() {
    static std::string features;
    if (!loaded_features) {
        loaded_features = true;

        features.clear();
        if (get_server_properties().get_bool("force-mob-spawning", DEFAULT_FORCE_MOB_SPAWNING)) {
            features += "Force Mob Spawning|";
        }
        if (get_server_properties().get_bool("death-messages", DEFAULT_DEATH_MESSAGES)) {
            features += "Implement Death Messages|";
        }
        if (get_server_properties().get_bool("generate-caves", DEFAULT_GENERATE_CAVES)) {
            features += "Generate Caves|";
        }
    }
    return features.c_str();
}

// Get Max Players
static unsigned char get_max_players() {
    int val = get_server_properties().get_int("max-players", DEFAULT_MAX_PLAYERS);
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
    std::ifstream properties_file(file);

    // Check Properties File
    if (!properties_file.good()) {
        // Write Defaults
        std::ofstream properties_file_output(file);
        properties_file_output << "# Message Of The Day\n";
        properties_file_output << "motd=" DEFAULT_MOTD "\n";
        properties_file_output << "# Show The MineCon Badge Next To MOTD In Server List\n";
        properties_file_output << "show-minecon-badge=" DEFAULT_SHOW_MINECON_BADGE "\n";
        properties_file_output << "# Game Mode (0 = Survival, 1 = Creative)\n";
        properties_file_output << "game-mode=" DEFAULT_GAME_MODE "\n";
        properties_file_output << "# Port\n";
        properties_file_output << "port=" DEFAULT_PORT "\n";
        properties_file_output << "# World Seed (Blank = Random Seed)\n";
        properties_file_output << "seed=" DEFAULT_SEED "\n";
        properties_file_output << "# Force Mob Spawning (false = Disabled, true = Enabled)\n";
        properties_file_output << "force-mob-spawning=" DEFAULT_FORCE_MOB_SPAWNING "\n";
        properties_file_output << "# Peaceful Mode (false = Disabled, true = Enabled)\n";
        properties_file_output << "peaceful-mode=" DEFAULT_PEACEFUL_MODE "\n";
        properties_file_output << "# World To Select\n";
        properties_file_output << "world-name=" DEFAULT_WORLD_NAME "\n";
        properties_file_output << "# Maximum Player Count\n";
        properties_file_output << "max-players=" DEFAULT_MAX_PLAYERS "\n";
        properties_file_output << "# Enable Whitelist\n";
        properties_file_output << "whitelist=" DEFAULT_WHITELIST "\n";
        properties_file_output << "# Enable Death Messages\n";
        properties_file_output << "death-messages=" DEFAULT_DEATH_MESSAGES "\n";
        properties_file_output << "# Generate Caves\n";
        properties_file_output << "generate-caves=" DEFAULT_GENERATE_CAVES "\n";
        properties_file_output.close();
        // Re-Open File
        properties_file = std::ifstream(file);
    }

    // Check Properties File
    if (!properties_file.is_open()) {
        ERR("Unable To Open %s", file.c_str());
    }
    // Load Properties
    get_server_properties().load(properties_file);
    // Close Properties File
    properties_file.close();

    // Create Empty Blacklist/Whitelist File
    std::string blacklist_file_path = get_blacklist_file();
    std::ifstream blacklist_file(blacklist_file_path);
    if (!blacklist_file.good()) {
        // Write Default
        std::ofstream blacklist_output(blacklist_file_path);
        blacklist_output << "# Blacklist/Whitelist; Each Line Is One IP Address\n";
        blacklist_output.close();
    }
    if (blacklist_file.is_open()) {
        blacklist_file.close();
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
    if (get_server_properties().get_bool("show-minecon-badge", DEFAULT_SHOW_MINECON_BADGE)) {
        unsigned char minecon_badge_patch[4] = {0x04, 0x1a, 0x9f, 0xe5}; // "ldr r1, [0x741f0]"
        patch((void *) 0x737e4, minecon_badge_patch);
    }

    // Log IPs
    overwrite_call((void *) 0x75e54, (void *) ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection);

    // Track TPS
    misc_run_on_tick(Minecraft_tick_injection);

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
