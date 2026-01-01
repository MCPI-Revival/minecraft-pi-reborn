#include <optional>

#include <libreborn/log.h>
#include <libreborn/util/string.h>

#include <mods/misc/misc.h>
#include <mods/compat/compat.h>
#include <mods/fps/fps.h>

#include "internal.h"

// Find Players With Username And Run Callback
typedef std::function<void(Minecraft *minecraft, const std::string &username, Player *player)> player_callback_t;
static void find_players(Minecraft *minecraft, const std::string &target_username, const player_callback_t &callback, const bool all_players) {
    const Level *level = minecraft->level;
    const std::vector<Player *> players = level->players;
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
static void ban_callback(MCPI_UNUSED Minecraft *minecraft, MCPI_UNUSED const std::string &username, Player *player) {
    blacklist.ban((ServerPlayer *) player);
}

// Kill Player
static void kill_callback(MCPI_UNUSED Minecraft *minecraft, const std::string &username, Player *player) {
    player->stopSleepInBed(true, true, false);
    player->health = 0;
    player->die(nullptr);
    INFO("Killed: %s", username.c_str());
}

// List Player
static void list_callback(const Minecraft *minecraft, const std::string &username, Player *player) {
    const RakNet_RakPeer *rak_peer = minecraft->rak_net_instance->peer;
    const RakNet_RakNetGUID guid = ((ServerPlayer *) player)->guid;
    INFO(" - %s (%s)", username.c_str(), get_rak_net_guid_ip(rak_peer, guid).c_str());
}

// Kick Player
void server_kick(const ServerPlayer *player) {
    const RakNet_RakNetGUID &guid = player->guid;
    RakNet_AddressOrGUID target = {};
    target.constructor(guid);
    player->level->rak_net_instance->peer->CloseConnection(&target, true, 0, HIGH_PRIORITY);
    player->minecraft->network_handler->onDisconnect(guid);
}
static void kick_callback(MCPI_UNUSED Minecraft *minecraft, const std::string &username, Player *player) {
    server_kick((ServerPlayer *) player);
    INFO("Kicked: %s", username.c_str());
}

// Built-In Commands
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
            std::string ip = cmd;
            trim(ip);
            blacklist.add_ip(ip);
        }
    });
    commands->push_back({
        .name = blacklist.is_white ? ban_ip_command : "pardon-ip ",
        .comment = std::string("Remove IP From ") + blacklist.get_name(true),
        .callback = [](const std::string &cmd) {
            std::string ip = cmd;
            trim(ip);
            blacklist.remove_ip(ip);
        }
    });

    // Reload White/Blacklist
    commands->push_back({
        .name = "reload",
        .comment = std::string("Reload The ") + blacklist.get_name(true),
        .callback = [](MCPI_UNUSED const std::string &cmd) {
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
            const std::string cpp_string = to_cp437(message);
            // Post Message To Chat
            server_side_network_handler->displayGameMessage(cpp_string);
        }
    });

    // List Players
    commands->push_back({
        .name = "list",
        .comment = "List All Players",
        .callback = [minecraft](MCPI_UNUSED const std::string &cmd) {
            INFO("All Players:");
            find_players(minecraft, "", list_callback, true);
        }
    });

    // Ticks-Per-Second
    commands->push_back({
        .name = "tps",
        .comment = "Print TPS",
        .callback = [](MCPI_UNUSED const std::string &cmd) {
            INFO("TPS: %f", tps);
        }
    });

    // Stop
    commands->push_back({
        .name = "stop",
        .comment = "Stop Server",
        .callback = [](MCPI_UNUSED const std::string &cmd) {
            compat_request_exit();
        }
    });

    // Time
    commands->push_back({
        .name = "get-time",
        .comment = "Get The Current Time (In Ticks)",
        .callback = [minecraft](MCPI_UNUSED const std::string &cmd) {
            INFO("Time: %i", minecraft->level->data.time);
            minecraft->level->data.time = 12000;
        }
    });
    commands->push_back({
        .name = "set-time ",
        .comment = "Set The Current Time",
        .callback = [minecraft](const std::string &cmd) {
            std::optional<int> new_time;
            if (cmd == "day") {
                new_time = 1000;
            } else if (cmd == "night") {
                new_time = 13000;
            } else {
                try {
                    const int x = std::stoi(cmd);
                    if (x >= 0) {
                        new_time = x;
                    }
                } catch (...) {}
            }
            if (new_time.has_value()) {
                const int x = new_time.value();
                minecraft->level->data.time = x;
                INFO("Set Time To: %i", x);
            } else {
                WARN("Invalid Time: %s", cmd.c_str());
            }
        }
    });

    // Help Page
    commands->push_back({
        .name = "help",
        .comment = "Print This Message",
        .callback = [commands](MCPI_UNUSED const std::string &cmd) {
            print_server_help(*commands);
        }
    });

    // Return
    return commands;
}