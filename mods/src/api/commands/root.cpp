#include "internal.h"

#include <libreborn/config.h>

#include <symbols/CommandServer.h>

#include <mods/misc/misc.h>

// Handle Root Package
std::string api_handle_command(const std::function<std::string()> &super, CommandServer *server, const ConnectedClient &client, std::string_view &cmd, std::istringstream &args) {
    // Manipulate The Level
    package(world) {
        return api_handle_world_command(super, server, cmd, args);
    }

    // Manipulate An Individual Entity
    package(entity) {
        return api_handle_entity_command(super, server, client, cmd, args);
    }

    // Chat
    package(chat) {
        passthrough(post);
    }

    // Manipulate The Camera
    package(camera) {
        package(mode) {
            passthrough(setFixed);
            passthrough(setNormal);
            passthrough(setFollow);
        }
        passthrough(setPos);
    }

    // Handle Events
    package(events) {
        return api_handle_event_command(server, client, cmd, std::nullopt);
    }

    // Extra Reborn Extensions
    package(reborn) {
        // Enable/Disable Compatibility Mode
        command(disableCompatMode) {
            api_compat_mode = false;
            return std::string(reborn_config.general.version) + '\n';
        }
        command(enableCompatMode) {
            api_compat_mode = true;
            return CommandServer::NullString;
        }
        // Control "Null Responses"
        command(sendNullResponses) {
            api_replace_null_responses = true;
            return CommandServer::NullString;
        }
        command(hideNullResponses) {
            api_replace_null_responses = false;
            return CommandServer::NullString;
        }
    }

    // Invalid Command
    return CommandServer::Fail;
}