#include <string>
#include <stdint.h>
#include <ctime>
#include <cstdio>
#include <csignal>
#include <fstream>
#include <vector>

#include <pthread.h>

#include <unistd.h>

#include <SDL/SDL_events.h>

#include <libreborn/libreborn.h>

#include "server_internal.h"
#include "server_properties.h"

#include "../feature/feature.h"
#include "../init/init.h"

#include <libreborn/minecraft.h>

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
#define DEFAULT_MOB_SPAWNING "true"
#define DEFAULT_PEACEFUL_MODE "false"
#define DEFAULT_WORLD_NAME "world"
#define DEFAULT_MAX_PLAYERS "4"

// Read STDIN Thread
static volatile bool stdin_buffer_complete = false;
static volatile char *stdin_buffer = NULL;
static void *read_stdin_thread(__attribute__((unused)) void *data) {
    while (1) {
        if (!stdin_buffer_complete) {
            int x = getchar();
            if (x != EOF) {
                if (x == '\n') {
                    if (stdin_buffer == NULL) {
                        stdin_buffer = strdup("");
                    }
                    stdin_buffer_complete = true;
                } else {
                    asprintf((char **) &stdin_buffer, "%s%c", stdin_buffer == NULL ? "" : stdin_buffer, (char) x);
                    ALLOC_CHECK(stdin_buffer);
                }
            }
        }
    }
}

// Get World Name
std::string server_internal_get_world_name() {
    return get_server_properties().get_string("world-name", DEFAULT_WORLD_NAME);
}

// Create/Start World
static void start_world(unsigned char *minecraft) {
    INFO("%s", "Starting Minecraft: Pi Edition Dedicated Server");

    LevelSettings settings;
    settings.game_type = get_server_properties().get_int("game-mode", DEFAULT_GAME_MODE);;
    std::string seed_str = get_server_properties().get_string("seed", DEFAULT_SEED);
    int32_t seed = seed_str.length() > 0 ? std::stoi(seed_str) : time(NULL);
    settings.seed = seed;

    std::string world_name = server_internal_get_world_name();
    (*Minecraft_selectLevel)(minecraft, world_name, world_name, settings);

    int port = get_server_properties().get_int("port", DEFAULT_PORT);
    (*Minecraft_hostMultiplayer)(minecraft, port);
    INFO("Listening On: %i", port);

    void *screen = ::operator new(PROGRESS_SCREEN_SIZE);
    ALLOC_CHECK(screen);
    screen = (*ProgressScreen)((unsigned char *) screen);
    (*Minecraft_setScreen)(minecraft, (unsigned char *) screen);
}

// Check If Two Percentages Are Different Enough To Be Logged
#define SIGNIFICANT_PROGRESS 5
static bool is_progress_difference_significant(int32_t new_val, int32_t old_val) {
    if (new_val != old_val) {
        if (new_val == -1 || old_val == -1) {
            return true;
        } else if (new_val == 0 || new_val == 100) {
            return true;
        } else {
            return new_val - old_val >= SIGNIFICANT_PROGRESS;
        }
    } else {
        return false;
    }
}

// Print Progress Reports
static int last_progress = -1;
static const char *last_message = NULL;
static void print_progress(unsigned char *minecraft) {
    const char *message = (*Minecraft_getProgressMessage)(minecraft);
    int32_t progress = *(int32_t *) (minecraft + Minecraft_progress_property_offset);
    if ((*Minecraft_isLevelGenerated)(minecraft)) {
        message = "Ready";
        progress = -1;
    }
    if (message != NULL) {
        bool message_different = message != last_message;
        bool progress_significant = is_progress_difference_significant(progress, last_progress);
        if (message_different || progress_significant) {
            if (progress != -1) {
                INFO("Status: %s: %i%%", message, progress);
            } else {
                INFO("Status: %s", message);
            }
            if (message_different) {
                last_message = message;
            }
            if (progress_significant) {
                last_progress = progress;
            }
        }
    }
}

static std::string get_banned_ips_file() {
    std::string file(getenv("HOME"));
    file.append("/.minecraft-pi/banned-ips.txt");
    return file;
}

typedef void (*player_callback_t)(unsigned char *minecraft, std::string username, unsigned char *player);

// Get Vector Of Players In Level
std::vector<unsigned char *> server_internal_get_players(unsigned char *level) {
    return *(std::vector<unsigned char *> *) (level + Level_players_property_offset);
}
// Get Player's Username
std::string server_internal_get_player_username(unsigned char *player) {
    return *(char **) (player + Player_username_property_offset);
}
// Get Level From Minecraft
unsigned char *server_internal_get_level(unsigned char *minecraft) {
    return *(unsigned char **) (minecraft + Minecraft_level_property_offset);
}
// Get minecraft from ServerPlayer
unsigned char *server_internal_get_minecraft(unsigned char *player) {
    return *(unsigned char **) (player + ServerPlayer_minecraft_property_offset);
}

// Find Players With Username And Run Callback
static void find_players(unsigned char *minecraft, std::string target_username, player_callback_t callback, bool all_players) {
    unsigned char *level = server_internal_get_level(minecraft);
    std::vector<unsigned char *> players = server_internal_get_players(level);
    bool found_player = false;
    for (std::size_t i = 0; i < players.size(); i++) {
        // Iterate Players
        unsigned char *player = players[i];
        std::string username = server_internal_get_player_username(player);
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

// Get IP From Player
static char *get_player_ip(unsigned char *minecraft, unsigned char *player) {
    RakNet_RakNetGUID guid = get_rak_net_guid(player);
    unsigned char *rak_peer = get_rak_peer(minecraft);
    RakNet_SystemAddress address = get_system_address(rak_peer, guid);
    // Get IP
    return (*RakNet_SystemAddress_ToString)(&address, false, '|');
}

// Ban Player
static void ban_callback(unsigned char *minecraft, std::string username, unsigned char *player) {
    // Get IP
    char *ip = get_player_ip(minecraft, player);

    // Ban Player
    INFO("Banned: %s (%s)", username.c_str(), ip);
    // Write To File
    std::ofstream banned_ips_output(get_banned_ips_file(), std::ios_base::app);
    if (banned_ips_output) {
        if (banned_ips_output.good()) {
            banned_ips_output << ip <<'\n';
        }
        if (banned_ips_output.is_open()) {
            banned_ips_output.close();
        }
    }
}

// Kill Player
static void kill_callback(__attribute__((unused)) unsigned char *minecraft, __attribute__((unused)) std::string username, unsigned char *player) {
    unsigned char *player_vtable = *(unsigned char **) player;
    Entity_die_t Entity_die = *(Entity_die_t *) (player_vtable + Entity_die_vtable_offset);
    (*Entity_die)(player, NULL);
    INFO("Killed: %s", username.c_str());
}

// List Player
static void list_callback(unsigned char *minecraft, std::string username, unsigned char *player) {
    INFO(" - %s (%s)", username.c_str(), get_player_ip(minecraft, player));
}

static void Level_saveLevelData_injection(unsigned char *level) {
    // Print Log Message
    INFO("%s", "Saving Game");

    // Call Original Method
    (*Level_saveLevelData)(level);
}

// Stop Server
static bool exit_requested = false;
static void exit_handler(__attribute__((unused)) int data) {
    exit_requested = true;
}
static void handle_server_stop(unsigned char *minecraft) {
    if (exit_requested) {
        INFO("%s", "Stopping Server");
        // Save And Exit
        unsigned char *level = server_internal_get_level(minecraft);
        if (level != NULL) {
            Level_saveLevelData_injection(level);
        }
        (*Minecraft_leaveGame)(minecraft, false);
        // Stop Game
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);

        exit_requested = false;
    }
}

// Get ServerSideNetworkHandler From Minecraft
static unsigned char *get_server_side_network_handler(unsigned char *minecraft) {
    return *(unsigned char **) (minecraft + Minecraft_network_handler_property_offset);
}

// Handle Commands
static void handle_commands(unsigned char *minecraft) {
    if ((*Minecraft_isLevelGenerated)(minecraft) && stdin_buffer_complete) {
        if (stdin_buffer != NULL) {
            unsigned char *server_side_network_handler = get_server_side_network_handler(minecraft);
            if (server_side_network_handler != NULL) {
                std::string data((char *) stdin_buffer);

                static std::string ban_command("ban ");
                static std::string say_command("say ");
                static std::string kill_command("kill ");
                static std::string list_command("list");
                static std::string stop_command("stop");
                static std::string help_command("help");
                if (data.rfind(ban_command, 0) == 0) {
                    // IP-Ban Target Username
                    std::string ban_username = data.substr(ban_command.length());
                    find_players(minecraft, ban_username, ban_callback, false);
                } else if (data.rfind(kill_command, 0) == 0) {
                    // Kill Target Username
                    std::string kill_username = data.substr(kill_command.length());
                    find_players(minecraft, kill_username, kill_callback, false);
                } else if (data.rfind(say_command, 0) == 0) {
                    // Format Message
                    std::string message = "[Server] " + data.substr(say_command.length());
                    // Post Message To Chat
                    (*ServerSideNetworkHandler_displayGameMessage)(server_side_network_handler, message);
                } else if (data == list_command) {
                    // List Players
                    INFO("%s", "All Players:");
                    find_players(minecraft, "", list_callback, true);
                } else if (data == stop_command) {
                    // Stop Server
                    exit_handler(-1);
                } else if (data == help_command) {
                    INFO("%s", "All Commands:");
                    INFO("%s", "    ban <Username>  - IP-Ban All Players With Specifed Username");
                    INFO("%s", "    kill <Username> - Kill All Players With Specifed Username");
                    INFO("%s", "    say <Message>   - Print Specified Message To Chat");
                    INFO("%s", "    list            - List All Players");
                    INFO("%s", "    stop            - Stop Server");
                    INFO("%s", "    help            - Print This Message");
                } else {
                    INFO("Invalid Command: %s", data.c_str());
                }
            }

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

    // Print Progress Reports
    print_progress(minecraft);

    // Call Original Method
    (*Minecraft_update)(minecraft);

    // Handle Commands
    handle_commands(minecraft);

    // Server Stop
    handle_server_stop(minecraft);
}

static bool RakNet_RakPeer_IsBanned_injection(__attribute__((unused)) unsigned char *rakpeer, const char *ip) {
    // Check banned-ips.txt
    std::string banned_ips_file_path = get_banned_ips_file();
    std::ifstream banned_ips_file(banned_ips_file_path);
    if (banned_ips_file) {
        bool ret = false;
        if (banned_ips_file.good()) {
            std::string line;
            while (std::getline(banned_ips_file, line)) {
                if (line.length() > 0) {
                    if (line[0] == '#') {
                        continue;
                    }
                    if (strcmp(line.c_str(), ip) == 0) {
                        ret = true;
                        break;
                    }
                }
            }
        }
        if (banned_ips_file.is_open()) {
            banned_ips_file.close();
        }
        return ret;
    } else {
        ERR("%s", "Unable To Read banned-ips.txt");
    }
}

static std::string get_motd() {
    std::string motd(get_server_properties().get_string("motd", DEFAULT_MOTD));
    return motd;
}

static bool loaded_features = false;
static const char *get_features() {
    static std::string features;
    if (!loaded_features) {
        loaded_features = true;

        features = "";
        if (get_server_properties().get_bool("spawn-mobs", DEFAULT_MOB_SPAWNING)) {
            features += "Mob Spawning|";
        }
        if (get_server_properties().get_bool("peaceful-mode", DEFAULT_PEACEFUL_MODE)) {
            features += "Peaceful Mode|";
        }
    }
    return features.c_str();
}

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

static void server_init() {
    // Open Properties File
    std::string file(getenv("HOME"));
    file.append("/.minecraft-pi/server.properties");

    std::ifstream properties_file(file);

    if (!properties_file || !properties_file.good()) {
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
        properties_file_output << "# Mob Spawning (false = Disabled, true = Enabled)\n";
        properties_file_output << "spawn-mobs=" DEFAULT_MOB_SPAWNING "\n";
        properties_file_output << "# Peaceful Mode (false = Disabled, true = Enabled)\n";
        properties_file_output << "peaceful-mode=" DEFAULT_PEACEFUL_MODE "\n";
        properties_file_output << "# World To Select\n";
        properties_file_output << "world-name=" DEFAULT_WORLD_NAME "\n";
        properties_file_output << "# Maximum Player Count\n";
        properties_file_output << "max-players=" DEFAULT_MAX_PLAYERS "\n";
        properties_file_output.close();
        // Re-Open File
        properties_file = std::ifstream(file);
    }

    if (!properties_file.is_open()) {
        ERR("%s", "Unable To Open server.properties");
    }

    // Load Properties
    get_server_properties().load(properties_file);

    properties_file.close();

    // Create Empty Banned IPs File
    std::string banned_ips_file_path = get_banned_ips_file();
    std::ifstream banned_ips_file(banned_ips_file_path);
    if (!banned_ips_file || !banned_ips_file.good()) {
        // Write Default
        std::ofstream banned_ips_output(banned_ips_file_path);
        banned_ips_output << "# List Of Banned IPs; Each Line Is One IP Address\n";
        banned_ips_output.close();
    }
    if (banned_ips_file.is_open()) {
        banned_ips_file.close();
    }

    // Prevent Main Player From Loading
    unsigned char player_patch[4] = {0x00, 0x20, 0xa0, 0xe3}; // "mov r2, #0x0"
    patch((void *) 0x1685c, player_patch);
    // Start World On Launch
    overwrite_calls((void *) Minecraft_update, (void *) Minecraft_update_injection);
    // Print Log On Game Save
    overwrite_calls((void *) Level_saveLevelData, (void *) Level_saveLevelData_injection);
    // Exit handler
    signal(SIGINT, exit_handler);
    signal(SIGTERM, exit_handler);
    // Set Max Players
    unsigned char max_players_patch[4] = {get_max_players(), 0x30, 0xa0, 0xe3}; // "mov r3, #MAX_PLAYERS"
    patch((void *) 0x166d0, max_players_patch);
    // Custom Banned IP List
    overwrite((void *) RakNet_RakPeer_IsBanned, (void *) RakNet_RakPeer_IsBanned_injection);

    if (get_server_properties().get_bool("show-minecon-badge", DEFAULT_SHOW_MINECON_BADGE)) {
        // Show The MineCon Icon Next To MOTD In Server List
        unsigned char minecon_badge_patch[4] = {0x04, 0x1a, 0x9f, 0xe5}; // "ldr r1, [0x741f0]"
        patch((void *) 0x737e4, minecon_badge_patch);
    }

    // Start Reading STDIN
    pthread_t read_stdin_thread_obj;
    pthread_create(&read_stdin_thread_obj, NULL, read_stdin_thread, NULL);
}

void init_server() {
    if (feature_get_mode() == 2) {
        server_init();
        setenv("MCPI_FEATURES", get_features(), 1);
        setenv("MCPI_USERNAME", get_motd().c_str(), 1);
    }
}