#include "internal.h"

#include <symbols/CommandServer.h>
#include <symbols/Minecraft.h>
#include <symbols/LocalPlayer.h>

#include <libreborn/patch.h>
#include <libreborn/util/string.h>

// Parse Commands
bool api_replace_null_responses = false;
static std::string CommandServer_parse_injection(CommandServer_parse_t original, CommandServer *server, ConnectedClient &client, const std::string &command) {
    // Parse Command
    std::string_view command_view = command;
    const size_t arg_start = command_view.find('(');
    if (arg_start == std::string::npos) {
        return CommandServer::Fail;
    }
    std::string_view cmd = command_view.substr(0, arg_start);
    const size_t cmd_end = command_view.rfind(')');
    if (cmd_end == std::string::npos) {
        return CommandServer::Fail;
    }
    std::string_view args_str = command_view.substr(arg_start + 1, cmd_end - arg_start - 1);

    // Support Passthrough To Vanilla's Implementation
    const std::function super = [original, server, &client, &command] {
        api_suppress_chat_events = true;
        const std::string ret = original(server, client, command);
        api_suppress_chat_events = false;
        return ret;
    };

    // Redirect Player Package To Entity
    std::string _new_cmd;
    std::string _new_args_str;
    package(player) {
        // The One Exception
        passthrough(setting);
        // Redirect All Other Commands
        const LocalPlayer *player = server->minecraft->player;
        if (player) {
            _new_cmd = package_str(entity) + std::string(cmd);
            cmd = _new_cmd;
            _new_args_str = safe_to_string(player->id) + arg_separator + std::string(args_str);
            args_str = _new_args_str;
        } else {
            return CommandServer::Fail;
        }
    }

    // Read Arguments
    std::istringstream args(std::string(args_str), std::ios::in);

    // Pass To Root Package
    std::string ret = api_handle_command(super, server, client, cmd, args);
    if (api_replace_null_responses && ret == CommandServer::NullString) {
        // Replace "Null Responses"
        ret = "Null\n";
    }

    // Return
    return ret;
}

// Init
void _init_api_commands() {
    overwrite_calls(CommandServer_parse, CommandServer_parse_injection);
    _init_api_events();
}