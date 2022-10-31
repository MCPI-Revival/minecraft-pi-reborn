// Config Needs To Load First
#include <libreborn/libreborn.h>

#ifndef MCPI_SERVER_MODE
#error "Server Code Requires Server Mode"
#endif

#include <string>
#include <stdint.h>
#include <ctime>
#include <cstdio>
#include <fstream>
#include <vector>

#include <sys/ioctl.h>
#include <pthread.h>

#include <unistd.h>

#include <SDL/SDL.h>

#include <symbols/minecraft.h>

#include <mods/server/server_properties.h>

#include <mods/feature/feature.h>
#include <mods/init/init.h>
#include <mods/home/home.h>
#include <mods/compat/compat.h>
#include <mods/misc/misc.h>

// --only-generate: Ony Generate World And Then Exit
static bool only_generate = false;
__attribute__((constructor)) static void _init_only_generate(int argc, char *argv[]) {
    // Iterate Arguments
    for (int i = 1; i < argc; i++) {
        // Check Argument
        if (strcmp(argv[i], "--only-generate") == 0) {
            // Enabled
            only_generate = true;
            break;
        }
    }
}

// Server Properties
static ServerProperties &get_server_properties() {
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
#define DEFAULT_DEATH_MESSAGES "false"

// Get World Name
static std::string get_world_name() {
    std::string name = get_server_properties().get_string("world-name", DEFAULT_WORLD_NAME);
    char *safe_name_c = to_cp437(name.c_str());
    std::string safe_name = safe_name_c;
    free(safe_name_c);
    return safe_name;
}

// Create/Start World
static void start_world(unsigned char *minecraft) {
    // Get World Name
    std::string world_name = get_world_name();

    // Log
    INFO("Loading World: %s", world_name.c_str());

    // Specify Level Settings
    LevelSettings settings;
    settings.game_type = get_server_properties().get_int("game-mode", DEFAULT_GAME_MODE);
    std::string seed_str = get_server_properties().get_string("seed", DEFAULT_SEED);
    int32_t seed = seed_str.length() > 0 ? std::stoi(seed_str) : time(NULL);
    settings.seed = seed;

    // Select Level
    (*Minecraft_selectLevel)(minecraft, world_name, world_name, settings);

    // Don't Open Port When Using --only-generate
    if (!only_generate) {
        // Open Port
        int port = get_server_properties().get_int("port", DEFAULT_PORT);
        INFO("Listening On: %i", port);
        patch_address((void *) 0x16cd8, (void *) port);
        (*Minecraft_hostMultiplayer)(minecraft, port);
    }

    // Open ProgressScreen
    unsigned char *screen = (unsigned char *) ::operator new(PROGRESS_SCREEN_SIZE);
    ALLOC_CHECK(screen);
    screen = (*ProgressScreen)(screen);
    (*Minecraft_setScreen)(minecraft, screen);
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
static std::vector<unsigned char *> get_players_in_level(unsigned char *level) {
    return *(std::vector<unsigned char *> *) (level + Level_players_property_offset);
}
// Get Player's Username
static std::string get_player_username(unsigned char *player) {
    std::string *username = (std::string *) (player + Player_username_property_offset);
    char *safe_username_c = from_cp437(username->c_str());
    std::string safe_username = safe_username_c;
    free(safe_username_c);
    return safe_username;
}
// Get Level From Minecraft
static unsigned char *get_level(unsigned char *minecraft) {
    return *(unsigned char **) (minecraft + Minecraft_level_property_offset);
}

// Find Players With Username And Run Callback
typedef void (*player_callback_t)(unsigned char *minecraft, std::string username, unsigned char *player);
static void find_players(unsigned char *minecraft, std::string target_username, player_callback_t callback, bool all_players) {
    unsigned char *level = get_level(minecraft);
    std::vector<unsigned char *> players = get_players_in_level(level);
    bool found_player = false;
    for (std::size_t i = 0; i < players.size(); i++) {
        // Iterate Players
        unsigned char *player = players[i];
        std::string username = get_player_username(player);
        if (all_players || username == target_username) {
            // Run Callback
            (*callback)(minecraft, username, player);
            found_player = true;
        }
    }
    if (!all_players && !found_player) {
        INFO("Invalid Player: %s", target_username.c_str());
    }
}

// Get RakNet Objects
static RakNet_RakNetGUID get_rak_net_guid(unsigned char *player) {
    return *(RakNet_RakNetGUID *) (player + ServerPlayer_guid_property_offset);
}
static RakNet_SystemAddress get_system_address(unsigned char *rak_peer, RakNet_RakNetGUID guid) {
    unsigned char *rak_peer_vtable = *(unsigned char **) rak_peer;
    RakNet_RakPeer_GetSystemAddressFromGuid_t RakNet_RakPeer_GetSystemAddressFromGuid = *(RakNet_RakPeer_GetSystemAddressFromGuid_t *) (rak_peer_vtable + RakNet_RakPeer_GetSystemAddressFromGuid_vtable_offset);
    // Get SystemAddress
    return (*RakNet_RakPeer_GetSystemAddressFromGuid)(rak_peer, guid);
}
static unsigned char *get_rak_peer(unsigned char *minecraft) {
    unsigned char *rak_net_instance = *(unsigned char **) (minecraft + Minecraft_rak_net_instance_property_offset);
    return *(unsigned char **) (rak_net_instance + RakNetInstance_peer_property_offset);
}
static char *get_rak_net_guid_ip(unsigned char *rak_peer, RakNet_RakNetGUID guid) {
    RakNet_SystemAddress address = get_system_address(rak_peer, guid);
    // Get IP
    return (*RakNet_SystemAddress_ToString)(&address, false, '|');
}

// Get IP From Player
static char *get_player_ip(unsigned char *minecraft, unsigned char *player) {
    unsigned char *rak_peer = get_rak_peer(minecraft);
    RakNet_RakNetGUID guid = get_rak_net_guid(player);
    // Return
    return get_rak_net_guid_ip(rak_peer,guid);
}

// Ban Player
static bool is_ip_in_blacklist(const char *ip);
static void ban_callback(unsigned char *minecraft, std::string username, unsigned char *player) {
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
    is_ip_in_blacklist(NULL);
}

// Kill Player
static void kill_callback(__attribute__((unused)) unsigned char *minecraft, __attribute__((unused)) std::string username, unsigned char *player) {
    unsigned char *player_vtable = *(unsigned char **) player;
    Mob_actuallyHurt_t Player_actuallyHurt = *(Mob_actuallyHurt_t *) (player_vtable + Mob_actuallyHurt_vtable_offset);
    (*Player_actuallyHurt)(player, INT32_MAX);
    INFO("Killed: %s", username.c_str());
}

// List Player
static void list_callback(unsigned char *minecraft, std::string username, unsigned char *player) {
    INFO(" - %s (%s)", username.c_str(), get_player_ip(minecraft, player));
}

// Handle Server Stop
static void handle_server_stop(unsigned char *minecraft) {
    if (compat_check_exit_requested()) {
        INFO("Stopping Server");
        // Save And Exit
        unsigned char *level = get_level(minecraft);
        if (level != NULL) {
            Level_saveLevelData_injection(level);
        }
        (*Minecraft_leaveGame)(minecraft, false);
        // Stop Game
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
    }
}

// Track TPS
#define NANOSECONDS_IN_SECOND 1000000000ll
static long long int get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long long int a = (long long int) ts.tv_nsec;
    long long int b = ((long long int) ts.tv_sec) * NANOSECONDS_IN_SECOND;
    return a + b;
}
static bool is_last_tick_time_set = false;
static long long int last_tick_time;
static double tps = 0;
static void Minecraft_tick_injection(__attribute__((unused)) unsigned char *minecraft) {
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
static unsigned char *get_server_side_network_handler(unsigned char *minecraft) {
    return *(unsigned char **) (minecraft + Minecraft_network_handler_property_offset);
}

// Read STDIN Thread
static volatile bool stdin_buffer_complete = false;
static volatile char *stdin_buffer = NULL;
static void *read_stdin_thread(__attribute__((unused)) void *data) {
    // Loop
    while (1) {
        int bytes_available;
        if (ioctl(fileno(stdin), FIONREAD, &bytes_available) == -1) {
            bytes_available = 0;
        }
        char buffer[bytes_available];
        bytes_available = read(fileno(stdin), (void *) buffer, bytes_available);
        for (int i = 0; i < bytes_available; i++) {
            if (!stdin_buffer_complete) {
                // Read Data
                char x = buffer[i];
                if (x == '\n') {
                    if (stdin_buffer == NULL) {
                        stdin_buffer = (volatile char *) malloc(1);
                        stdin_buffer[0] = '\0';
                    }
                    stdin_buffer_complete = true;
                } else {
                    string_append((char **) &stdin_buffer, "%c", (char) x);
                }
            }
        }
    }
    return NULL;
}
__attribute__((destructor)) static void _free_stdin_buffer() {
    if (stdin_buffer != NULL) {
        free((void *) stdin_buffer);
        stdin_buffer = NULL;
    }
}
// Prevent STDIN From Being Closed
HOOK(close, int, (int fd)) {
    if (fd != fileno(stdin)) {
        ensure_close();
        return (*real_close)(fd);
    } else {
        return 0;
    }
}

// Handle Commands
static void handle_commands(unsigned char *minecraft) {
    // Check If Level Is Generated
    if ((*Minecraft_isLevelGenerated)(minecraft) && stdin_buffer_complete) {
        // Command Ready; Run It
        if (stdin_buffer != NULL) {
            unsigned char *server_side_network_handler = get_server_side_network_handler(minecraft);
            if (server_side_network_handler != NULL) {
                std::string data((char *) stdin_buffer);

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
                    is_ip_in_blacklist(NULL);
                } else if (data.rfind(kill_command, 0) == 0) {
                    // Kill Target Username
                    std::string kill_username = data.substr(kill_command.length());
                    find_players(minecraft, kill_username, kill_callback, false);
                } else if (data.rfind(say_command, 0) == 0) {
                    // Format Message
                    std::string message = "[Server] " + data.substr(say_command.length());
                    char *safe_message = to_cp437(message.c_str());
                    // Post Message To Chat
                    (*ServerSideNetworkHandler_displayGameMessage)(server_side_network_handler, safe_message);
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

            // Free
            free((void *) stdin_buffer);
            stdin_buffer = NULL;
        }
        stdin_buffer_complete = false;
    }
}

// Runs Every Tick
static bool loaded = false;
static void Minecraft_update_injection(unsigned char *minecraft) {
    // Create/Start World
    if (!loaded) {
        start_world(minecraft);
        loaded = true;
    }

    // Handle --only-generate
    if (only_generate && (*Minecraft_isLevelGenerated)(minecraft)) {
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
    if (ip == NULL) {
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
            if (x.compare(ip) == 0) {
                return true;
            }
        }
        return false;
    }
}

// Ban Players
static bool RakNet_RakPeer_IsBanned_injection(__attribute__((unused)) unsigned char *rakpeer, const char *ip) {
    // Check List
    bool ret = is_ip_in_blacklist(ip);
    if (is_whitelist()) {
        return !ret;
    } else {
        return ret;
    }
}

// Log IPs
static unsigned char *ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection(unsigned char *server_side_network_handler, RakNet_RakNetGUID *guid) {
    // Call Original Method
    unsigned char *player = (*ServerSideNetworkHandler_popPendingPlayer)(server_side_network_handler, guid);

    // Check If Player Is Null
    if (player != NULL) {
        // Get Data
        std::string *username = (std::string *) (player + Player_username_property_offset);
        unsigned char *minecraft = *(unsigned char **) (server_side_network_handler + ServerSideNetworkHandler_minecraft_property_offset);
        unsigned char *rak_peer = get_rak_peer(minecraft);
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
        if (get_server_properties().get_bool("peaceful-mode", DEFAULT_PEACEFUL_MODE)) {
            features += "Peaceful Mode|";
        }
        if (get_server_properties().get_bool("force-mob-spawning", DEFAULT_FORCE_MOB_SPAWNING)) {
            features += "Force Mob Spawning|";
        }
        if (get_server_properties().get_bool("death-messages", DEFAULT_DEATH_MESSAGES)) {
            features += "Implement Death Messages|";
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
    is_ip_in_blacklist(NULL);

    // Prevent Main Player From Loading
    unsigned char player_patch[4] = {0x00, 0x20, 0xa0, 0xe3}; // "mov r2, #0x0"
    patch((void *) 0x16ef8, player_patch);
    // Start World On Launch
    misc_run_on_update(Minecraft_update_injection);
    // Set Max Players
    unsigned char max_players_patch[4] = {get_max_players(), 0x30, 0xa0, 0xe3}; // "mov r3, #MAX_PLAYERS"
    patch((void *) 0x16cc0, max_players_patch);
    // Custom Banned IP List
    overwrite((void *) RakNet_RakPeer_IsBanned, (void *) RakNet_RakPeer_IsBanned_injection);

    // Show The MineCon Icon Next To MOTD In Server List
    if (get_server_properties().get_bool("show-minecon-badge", DEFAULT_SHOW_MINECON_BADGE)) {
        unsigned char minecon_badge_patch[4] = {0x5c, 0x1a, 0x9f, 0xe5}; // "ldr r1, [0x98af0]"
        patch((void *) 0x9808c, minecon_badge_patch);
    }

    // Log IPs
    overwrite_call((void *) 0x9a63c, (void *) ServerSideNetworkHandler_onReady_ClientGeneration_ServerSideNetworkHandler_popPendingPlayer_injection);

    // Track TPS
    misc_run_on_tick(Minecraft_tick_injection);

    // Start Reading STDIN
    pthread_t read_stdin_thread_obj;
    pthread_create(&read_stdin_thread_obj, NULL, read_stdin_thread, NULL);
}

// Init Server
void init_server() {
    server_init();
    set_and_print_env("MCPI_FEATURE_FLAGS", get_features());
    set_and_print_env("MCPI_RENDER_DISTANCE", "Tiny");
    set_and_print_env("MCPI_USERNAME", get_motd().c_str());
}
