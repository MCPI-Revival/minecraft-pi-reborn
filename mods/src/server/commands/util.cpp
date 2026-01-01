#include <unistd.h>
#include <sys/ioctl.h>

#include <libreborn/log.h>

#include "internal.h"

// Command Parsing
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

// Read STDIN
static std::string check_stdin() {
    FILE *file = stdin;
    const int fd = fileno(file);
    std::string out;
    if (isatty(fd)) {
        int available_bytes = 0;
        const int ret = ioctl(fd, FIONREAD, &available_bytes);
        if (ret == 0 && available_bytes > 0) {
            char *line = nullptr;
            size_t len = 0;
            if (getline(&line, &len, file) > 0) {
                out = line;
            }
            free(line);
        }
    }
    return out;
}

// Handle Commands
void handle_commands(Minecraft *minecraft) {
    // Check If Level Is Generated
    if (!minecraft->isLevelGenerated()) {
        return;
    }
    // Read Line
    std::string data = check_stdin();
    if (data.empty()) {
        return;
    }
    data.pop_back(); // Remove Newline

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

// Help
void print_server_help(const std::vector<ServerCommand> &commands) {
    INFO("All Commands:");
    std::string::size_type max_lhs_length = 0;
    for (const ServerCommand &command : commands) {
        const std::string::size_type lhs_length = command.get_lhs_help().length();
        if (lhs_length > max_lhs_length) {
            max_lhs_length = lhs_length;
        }
    }
    for (const ServerCommand &command : commands) {
        INFO("%s", command.get_full_help(max_lhs_length).c_str());
    }
}