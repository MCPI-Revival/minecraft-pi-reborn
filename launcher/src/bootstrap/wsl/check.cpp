#include "../bootstrap.h"
#include "../../util/util.h"

#include <sstream>

#include <libreborn/util/exec.h>
#include <libreborn/util/string.h>
#include <libreborn/log.h>

// Run Command
static std::string run(const std::vector<const char *> &args, const bool output_is_wstring) {
    // Run
    const CommandResult result = run_command(args.data());
    // Get Output
    std::string ret;
    if (!result.output.empty() && is_exit_status_success(result.status)) {
        // Convert If Needed
        if (output_is_wstring) {
            // Convert To UTF-8
            const std::wstring str = result.str<std::wstring>();
            ret = convert_wstring_to_utf8(str);
            // Remove CR Line Endings
            std::erase(ret, '\r');
        } else {
            // Assume Correct Encoding
            ret = result.str<std::string>();
        }
    }
    // Return
    return ret;
}

// List Container
static std::string read_field(std::string_view &str) {
    // Find End Of Current Field
    constexpr char delimiter = ' ';
    const std::string_view::size_type current_field_end = str.find_first_of(delimiter);
    if (current_field_end == std::string::npos) {
        // Last/Only Field
        const std::string_view ret = str;
        str = {};
        return std::string(ret);
    }
    // Find Start Of Next Field
    std::string_view::size_type next_field_start = str.find_first_not_of(delimiter, current_field_end + 1);
    if (next_field_start == std::string::npos) {
        // No Next Field
        next_field_start = str.size();
    }
    // Extract Field
    const std::string_view ret = str.substr(0, current_field_end);
    str.remove_prefix(next_field_start);
    return std::string(ret);
}
struct Container {
    // Constructor
    explicit Container(std::string_view str) {
        is_default = !read_field(str).empty();
        name = read_field(str);
        state = read_field(str);
        version = read_field(str);
    }
    // Properties
    bool is_default;
    std::string name;
    std::string state;
    std::string version;
};
static std::vector<Container> list_wsl_containers() {
    // Run Command
    const std::vector<const char *> args = {
        "wsl",
        "--list",
        "--verbose",
        nullptr
    };
    const std::string str = run(args, true);

    // Parse
    std::vector<Container> ret;
    std::stringstream stream(str);
    std::string line;
    bool is_table_header = true;
    while (std::getline(stream, line)) {
        // Skip Header
        if (is_table_header) {
            is_table_header = false;
            continue;
        }
        // Parse Line
        if (!line.empty()) {
            ret.emplace_back(line);
        }
    }
    return ret;
}

// Check WSL
static bool is_wsl_working() {
    // List Containers
    DEBUG("Listing WSL Containers...");
    const std::vector<Container> containers = list_wsl_containers();
    bool found = false;
    for (const Container &container : containers) {
        DEBUG("- Default: [%s], Name: [%s], State: [%s], Version: [%s]", container.is_default ? "Yes" : "No", container.name.c_str(), container.state.c_str(), container.version.c_str());
        if (container.name == WSL_CONTAINER_NAME && container.version == "1") {
            found = true;
        }
    }
    if (!found) {
        return false;
    }

    // Test Container
    const std::string magic = "Hello World!";
    const std::vector<const char *> args = {
        "wsl",
        WSL_FLAGS,
        "--exec", "echo", magic.c_str(),
        nullptr
    };
    std::string str = run(args, false);
    trim(str);
    return str == magic;
}
void check_wsl() {
    const bool ret = is_wsl_working();
    if (!ret) {
        user_error("Unable to connect to WSL container. Have you followed the getting started guide?");
    }
}