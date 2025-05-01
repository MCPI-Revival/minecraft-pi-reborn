#include <fstream>
#include <cstdint>
#include <algorithm>

#include <libreborn/log.h>
#include <libreborn/util/string.h>

#include <symbols/minecraft.h>

#include <mods/misc/misc.h>
#include <mods/compat/compat.h>
#include <mods/fps/fps.h>
#include <mods/server/server.h>

#include "internal.h"

// Get Vector Of Players In Level
static std::vector<Player *> get_players_in_level(Level *level) {
    return level->players;
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
    bool found_player = all_players;
    for (Player *player : players) {
        // Iterate Players
        std::string username = misc_get_player_username_utf(player);
        if (all_players || username == target_username) {
            // Run Callback
            callback(minecraft, username, player);
            found_player = true;
        }
    }
    if (!found_player) {
        WARN("Invalid Player: %s", target_username.c_str());
    }
}

// Ban Player
static void ban_callback(__attribute__((unused)) Minecraft *minecraft, __attribute__((unused)) const std::string &username, Player *player) {
    blacklist.ban((ServerPlayer *) player);
}

// Kill Player
static void kill_callback(__attribute__((unused)) Minecraft *minecraft, const std::string &username, Player *player) {
    player->hurt(nullptr, INT32_MAX);
    INFO("Killed: %s", username.c_str());
}

// List Player
static void list_callback(Minecraft *minecraft, const std::string &username, Player *player) {
    RakNet_RakPeer *rak_peer = minecraft->rak_net_instance->peer;
    const RakNet_RakNetGUID guid = ((ServerPlayer *) player)->guid;
    INFO(" - %s (%s)", username.c_str(), get_rak_net_guid_ip(rak_peer, guid).c_str());
}

// Kick Player
void server_kick(const ServerPlayer *player) {
    const RakNet_RakNetGUID &guid = player->guid;
    RakNet_AddressOrGUID target;
    target.constructor(guid);
    player->level->rak_net_instance->peer->CloseConnection(&target, true, 0, LOW_PRIORITY);
    player->minecraft->network_handler->onDisconnect(guid);
}
static void kick_callback(__attribute__((unused)) Minecraft *minecraft, const std::string &username, Player *player) {
    server_kick((ServerPlayer *) player);
    INFO("Kicked: %s", username.c_str());
}

// Trim String
static void ltrim(std::string &s) {
    s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) {
        return !std::isspace(ch);
    }));
}
static void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](const unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
static std::string trim(const std::string &x) {
    std::string s = x;
    rtrim(s);
    ltrim(s);
    return s;
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
void start_reading_commands() {
    pthread_create(&read_stdin_thread_obj, nullptr, read_stdin_thread, nullptr);
}
void stop_reading_commands() {
    pthread_cancel(read_stdin_thread_obj);
    pthread_join(read_stdin_thread_obj, nullptr);
    stdin_line_ready = false;
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
    commands->push_back({
        .name = "ban ",
        .comment = "IP-Ban All Players With Specified Username",
        .callback = [minecraft](const std::string &cmd) {
            find_players(minecraft, cmd, ban_callback, false);
        }
    });
    const std::string ban_ip_command = "ban-ip ";
    commands->push_back({
        .name = blacklist.is_white ? blacklist.get_name(false) + "-ip " : ban_ip_command,
        .comment = std::string("Add IP To ") + blacklist.get_name(true),
        .callback = [](const std::string &cmd) {
            const std::string ip = trim(cmd);
            blacklist.add_ip(ip);
        }
    });
    commands->push_back({
        .name = blacklist.is_white ? ban_ip_command : "pardon-ip ",
        .comment = std::string("Remove IP From ") + blacklist.get_name(true),
        .callback = [](const std::string &cmd) {
            const std::string ip = trim(cmd);
            blacklist.remove_ip(ip);
        }
    });

    // Reload White/Blacklist
    commands->push_back({
        .name = "reload",
        .comment = std::string("Reload The ") + blacklist.get_name(true),
        .callback = [](__attribute__((unused)) const std::string &cmd) {
            INFO("Reloading %s", blacklist.get_name(true).c_str());
            blacklist.load();
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

    // Kick Player
    commands->push_back({
        .name = "kick ",
        .comment = "Kick All Players With Specified Username",
        .callback = [minecraft](const std::string &cmd) {
            find_players(minecraft, cmd, kick_callback, false);
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
void handle_commands(Minecraft *minecraft) {
    // Check If Level Is Generated
    if (minecraft->isLevelGenerated() && stdin_line_ready) {
        // Read Line
        std::string data = std::move(stdin_line);
        data.pop_back(); // Remove Newline
        stdin_line_ready = false;
        // Command Ready; Run It
        ServerSideNetworkHandler *server_side_network_handler = (ServerSideNetworkHandler *) minecraft->network_handler;
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
                WARN("Invalid Command: %s", data.c_str());
            }
            // Free
            delete commands;
        }
    }
}