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

#include <libcore/libcore.h>

#include "server.h"
#include "server_properties.h"

// Server Properties
static ServerProperties &get_server_properties() {
    static ServerProperties properties;
    return properties;
}

// Default Server Properties
#define DEFAULT_MOTD "Minecraft Server"
#define DEFAULT_SHOW_MINECON_ICON "false"
#define DEFAULT_GAME_MODE "0"
#define DEFAULT_PORT "19132"
#define DEFAULT_SEED ""
#define DEFAULT_MOB_SPAWNING "true"
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
                    if (stdin_buffer == NULL) {
                        asprintf((char **) &stdin_buffer, "%c", (char) x);
                    } else {
                        asprintf((char **) &stdin_buffer, "%s%c", stdin_buffer, (char) x);
                    }
                }
            }
        }
    }
}

typedef void (*Minecraft_update_t)(unsigned char *minecraft);
static Minecraft_update_t Minecraft_update = (Minecraft_update_t) 0x16b74;
static void *Minecraft_update_original = NULL;

struct LevelSettings {
    unsigned long seed;
    int32_t game_type;
};

typedef void (*Minecraft_selectLevel_t)(unsigned char *minecraft, std::string const& level_dir, std::string const& level_name, LevelSettings const& vsettings);
static Minecraft_selectLevel_t Minecraft_selectLevel = (Minecraft_selectLevel_t) 0x16f38;

typedef void (*Minecraft_hostMultiplayer_t)(unsigned char *minecraft, int32_t port);
static Minecraft_hostMultiplayer_t Minecraft_hostMultiplayer = (Minecraft_hostMultiplayer_t) 0x16664;

typedef void *(*ProgressScreen_t)(unsigned char *obj);
static ProgressScreen_t ProgressScreen = (ProgressScreen_t) 0x37044;

typedef void (*Minecraft_setScreen_t)(unsigned char *minecraft, unsigned char *screen);
static Minecraft_setScreen_t Minecraft_setScreen = (Minecraft_setScreen_t) 0x15d6c;

// Create/Start World
static void start_world(unsigned char *minecraft) {
    INFO("%s", "Starting Minecraft: Pi Edition Dedicated Server");

    LevelSettings settings;
    settings.game_type = get_server_properties().get_int("game-mode", DEFAULT_GAME_MODE);;
    std::string seed_str = get_server_properties().get_string("seed", DEFAULT_SEED);
    int32_t seed = seed_str.length() > 0 ? std::stoi(seed_str) : time(NULL);
    settings.seed = seed;

    std::string world_name = get_server_properties().get_string("world-name", DEFAULT_WORLD_NAME);
    (*Minecraft_selectLevel)(minecraft, world_name, world_name, settings);

    int port = get_server_properties().get_int("port", DEFAULT_PORT);
    (*Minecraft_hostMultiplayer)(minecraft, port);
    INFO("Listening On: %i", port);

    void *screen = ::operator new(0x4c);
    screen = (*ProgressScreen)((unsigned char *) screen);
    (*Minecraft_setScreen)(minecraft, (unsigned char *) screen);
}

typedef const char *(*Minecraft_getProgressMessage_t)(unsigned char *minecraft);
static Minecraft_getProgressMessage_t Minecraft_getProgressMessage = (Minecraft_getProgressMessage_t) 0x16e58;

typedef int32_t (*Minecraft_isLevelGenerated_t)(unsigned char *minecraft);
static Minecraft_isLevelGenerated_t Minecraft_isLevelGenerated = (Minecraft_isLevelGenerated_t) 0x16e6c;

#define SIGNIFICANT_PROGRESS 5

// Check If Two Percentages Are Different Enough To Be Logged
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
    int32_t progress = *(int32_t *) (minecraft + 0xc60);
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

struct RakNet_RakNetGUID {
    unsigned char data[10];
};
struct RakNet_SystemAddress {
    unsigned char data[20];
};
typedef RakNet_SystemAddress (*RakNet_RakPeer_GetSystemAddressFromGuid_t)(unsigned char *rak_peer, RakNet_RakNetGUID guid);

typedef void (*ServerSideNetworkHandler_displayGameMessage_t)(unsigned char *server_side_network_handler, std::string const& message);
static ServerSideNetworkHandler_displayGameMessage_t ServerSideNetworkHandler_displayGameMessage = (ServerSideNetworkHandler_displayGameMessage_t) 0x750c4;

typedef char *(*RakNet_SystemAddress_ToString_t)(RakNet_SystemAddress *system_address, bool print_delimiter, char delimiter);
static RakNet_SystemAddress_ToString_t RakNet_SystemAddress_ToString = (RakNet_SystemAddress_ToString_t) 0xd6198;

static std::string get_banned_ips_file() {
    std::string file(getenv("HOME"));
    file.append("/.minecraft/banned-ips.txt");
    return file;
}

typedef void (*player_callback_t)(unsigned char *minecraft, std::string username, unsigned char *player);

// Find Players With Username And Run Callback
static void find_players(unsigned char *minecraft, std::string target_username, player_callback_t callback, bool all_players) {
    unsigned char *level = *(unsigned char **) (minecraft + 0x188);
    std::vector<unsigned char *> players = *(std::vector<unsigned char *> *) (level + 0x60);
    bool found_player = false;
    for (std::size_t i = 0; i < players.size(); i++) {
        // Iterate Players
        unsigned char *player = players[i];
        std::string username = *(std::string *) (player + 0xbf4);
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

static RakNet_RakNetGUID *get_rak_net_guid(unsigned char *player) {
    return (RakNet_RakNetGUID *) (player + 0xc08);
}
static RakNet_SystemAddress get_system_address(unsigned char *rak_peer, RakNet_RakNetGUID guid) {
    unsigned char *rak_peer_vtable = *(unsigned char **) rak_peer;
    RakNet_RakPeer_GetSystemAddressFromGuid_t RakNet_RakPeer_GetSystemAddressFromGuid = *(RakNet_RakPeer_GetSystemAddressFromGuid_t *) (rak_peer_vtable + 0xd0);
    // Get SystemAddress
    return (*RakNet_RakPeer_GetSystemAddressFromGuid)(rak_peer, guid);
}
static unsigned char *get_rak_peer(unsigned char *minecraft) {
    unsigned char *rak_net_instance = *(unsigned char **) (minecraft + 0x170);
    return *(unsigned char **) (rak_net_instance + 0x4);
}

// Get IP From Player
static char *get_player_ip(unsigned char *minecraft, unsigned char *player) {
    RakNet_RakNetGUID guid = *get_rak_net_guid(player);
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
typedef void (*Entity_die_t)(unsigned char *entity, unsigned char *cause);
static void kill_callback(__attribute__((unused)) unsigned char *minecraft, __attribute__((unused)) std::string username, unsigned char *player) {
    unsigned char *player_vtable = *(unsigned char **) player;
    Entity_die_t Entity_die = *(Entity_die_t *) (player_vtable + 0x130);
    (*Entity_die)(player, NULL);
    INFO("Killed: %s", username.c_str());
}

// List Player
static void list_callback(unsigned char *minecraft, std::string username, unsigned char *player) {
    INFO(" - %s (%s)", username.c_str(), get_player_ip(minecraft, player));
}

typedef void (*Level_saveLevelData_t)(unsigned char *level);
static Level_saveLevelData_t Level_saveLevelData = (Level_saveLevelData_t) 0xa2e94;
static void *Level_saveLevelData_original = NULL;

static void Level_saveLevelData_injection(unsigned char *level) {
    // Print Log Message
    INFO("%s", "Saving Game");

    // Call Original Method
    revert_overwrite((void *) Level_saveLevelData, Level_saveLevelData_original);
    (*Level_saveLevelData)(level);
    revert_overwrite((void *) Level_saveLevelData, Level_saveLevelData_original);
}

typedef void (*Minecraft_leaveGame_t)(unsigned char *minecraft, bool save_remote_level);
static Minecraft_leaveGame_t Minecraft_leaveGame = (Minecraft_leaveGame_t) 0x15ea0;

// Stop Server
static bool exit_requested = false;
static void exit_handler(__attribute__((unused)) int data) {
    exit_requested = true;
}
static void handle_server_stop(unsigned char *minecraft) {
    if (exit_requested) {
        INFO("%s", "Stopping Server");
        // Save And Exit
        unsigned char *level = *(unsigned char **) (minecraft + 0x188);
        (*Level_saveLevelData)(level);
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
    return *(unsigned char **) (minecraft + 0x174);
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
    revert_overwrite((void *) Minecraft_update, Minecraft_update_original);
    (*Minecraft_update)(minecraft);
    revert_overwrite((void *) Minecraft_update, Minecraft_update_original);

    // Handle Commands
    handle_commands(minecraft);

    // Server Stop
    handle_server_stop(minecraft);
}

typedef void (*Gui_addMessage_t)(unsigned char *gui, std::string const& text);
static Gui_addMessage_t Gui_addMessage = (Gui_addMessage_t) 0x27820;
static void *Gui_addMessage_original = NULL;

static void Gui_addMessage_injection(unsigned char *gui, std::string const& text) {
    // Print Log Message
    fprintf(stderr, "[CHAT]: %s\n", text.c_str());

    // Call Original Method
    revert_overwrite((void *) Gui_addMessage, Gui_addMessage_original);
    (*Gui_addMessage)(gui, text);
    revert_overwrite((void *) Gui_addMessage, Gui_addMessage_original);
}

typedef bool (*RakNet_RakPeer_IsBanned_t)(unsigned char *rakpeer, const char *ip);
static RakNet_RakPeer_IsBanned_t RakNet_RakPeer_IsBanned = (RakNet_RakPeer_IsBanned_t) 0xda3b4;
static void *RakNet_RakPeer_IsBanned_original = NULL;

static bool RakNet_RakPeer_IsBanned_injection(unsigned char *rakpeer, const char *ip) {
    // Call Original
    revert_overwrite((void *) RakNet_RakPeer_IsBanned, RakNet_RakPeer_IsBanned_original);
    bool ret = (*RakNet_RakPeer_IsBanned)(rakpeer, ip);
    revert_overwrite((void *) RakNet_RakPeer_IsBanned, RakNet_RakPeer_IsBanned_original);

    if (ret) {
        return true;
    } else {
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
}

const char *server_get_motd() {
    return get_server_properties().get_string("motd", DEFAULT_MOTD).c_str();
}
int server_get_mob_spawning() {
    return get_server_properties().get_bool("spawn-mobs", DEFAULT_MOB_SPAWNING);
}
static unsigned char server_get_max_players() {
    int val = get_server_properties().get_int("max-players", DEFAULT_MAX_PLAYERS);
    if (val < 0) {
        val = 0;
    }
    if (val > 255) {
        val = 255;
    }
    return (unsigned char) val;
}

void server_init() {
    // Open Properties File
    std::string file(getenv("HOME"));
    file.append("/.minecraft/server.properties");

    std::ifstream properties_file(file);

    if (!properties_file || !properties_file.good()) {
        // Write Defaults
        std::ofstream properties_file_output(file);
        properties_file_output << "# Message Of The Day\n";
        properties_file_output << "motd=" DEFAULT_MOTD "\n";
        properties_file_output << "# Show The MineCon Icon Next To MOTD In Server List\n";
        properties_file_output << "show-minecon-icon=" DEFAULT_SHOW_MINECON_ICON "\n";
        properties_file_output << "# Game Mode (0 = Survival, 1 = Creative)\n";
        properties_file_output << "game-mode=" DEFAULT_GAME_MODE "\n";
        properties_file_output << "# Port\n";
        properties_file_output << "port=" DEFAULT_PORT "\n";
        properties_file_output << "# World Seed (Blank = Random Seed)\n";
        properties_file_output << "seed=" DEFAULT_SEED "\n";
        properties_file_output << "# Mob Spawning (false = Disabled, true = Enabled)\n";
        properties_file_output << "spawn-mobs=" DEFAULT_MOB_SPAWNING "\n";
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
    unsigned char player_patch[4] = {0x00, 0x20, 0xa0, 0xe3};
    patch((void *) 0x1685c, player_patch);
    // Start World On Launch
    Minecraft_update_original = overwrite((void *) Minecraft_update, (void *) Minecraft_update_injection);
    // Print Log On Game Save
    Level_saveLevelData_original = overwrite((void *) Level_saveLevelData, (void *) Level_saveLevelData_injection);
    // Exit handler
    signal(SIGINT, exit_handler);
    // Print Chat To Log
    Gui_addMessage_original = overwrite((void *) Gui_addMessage, (void *) Gui_addMessage_injection);
    // Allow All IPs To Join
    unsigned char allow_all_ip_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
    patch((void *) 0xe1f6c, allow_all_ip_patch);
    // Set Max Players
    unsigned char max_players_patch[4] = {server_get_max_players(), 0x30, 0xa0, 0xe3};
    patch((void *) 0x166d0, max_players_patch);
    // Custom Banned IP List
    RakNet_RakPeer_IsBanned_original = overwrite((void *) RakNet_RakPeer_IsBanned, (void *) RakNet_RakPeer_IsBanned_injection);

    if (get_server_properties().get_bool("show-minecon-icon", DEFAULT_SHOW_MINECON_ICON)) {
        // Show The MineCon Icon Next To MOTD In Server List
        unsigned char minecon_icon_patch[4] = {0x04, 0x1a, 0x9f, 0xe5};
        patch((void *) 0x737e4, minecon_icon_patch);
    }

    // Start Reading STDIN
    pthread_t read_stdin_thread_obj;
    pthread_create(&read_stdin_thread_obj, NULL, read_stdin_thread, NULL);
}
