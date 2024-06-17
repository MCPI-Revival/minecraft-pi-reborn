#include <sstream>
#include <cstring>
#include <cerrno>
#include <sys/wait.h>
#include <sys/stat.h>
#include <vector>
#include <functional>
#include <algorithm>

#include <libreborn/libreborn.h>

#include "../util.h"
#include "configuration.h"
#include "cache.h"

// Strip Feature Flag Default
std::string strip_feature_flag_default(const std::string &flag, bool *default_ret) {
    // Valid Values
    std::string true_str = "TRUE ";
    std::string false_str = "FALSE ";
    // Test
    if (flag.rfind(true_str, 0) == 0) {
        // Enabled By Default
        if (default_ret != nullptr) {
            *default_ret = true;
        }
        return flag.substr(true_str.length(), std::string::npos);
    } else if (flag.rfind(false_str, 0) == 0) {
        // Disabled By Default
        if (default_ret != nullptr) {
            *default_ret = false;
        }
        return flag.substr(false_str.length(), std::string::npos);
    } else {
        // Invalid
        ERR("Invalid Feature Flag Default");
    }
}

// Load Available Feature Flags
extern unsigned char available_feature_flags[];
extern size_t available_feature_flags_len;
void load_available_feature_flags(const std::function<void(std::string)> &callback) {
    // Load Data
    const std::string data(available_feature_flags, available_feature_flags + available_feature_flags_len);
    std::stringstream stream(data);
    // Store Lines
    std::vector<std::string> lines;
    // Read File
    {
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty()) {
                // Verify Line
                if (line.find('|') == std::string::npos) {
                    lines.push_back(line);
                } else {
                    // Invalid Line
                    ERR("Feature Flag Contains Invalid '|'");
                }
            }
        }
    }
    // Sort
    std::sort(lines.begin(), lines.end(), [](const std::string &a, const std::string &b) {
        // Strip Defaults
        const std::string stripped_a = strip_feature_flag_default(a, nullptr);
        const std::string stripped_b = strip_feature_flag_default(b, nullptr);
        // Sort
        return stripped_a < stripped_b;
    });
    // Run Callbacks
    for (const std::string &line : lines) {
        callback(line);
    }
}

// Run Command And Set Environmental Variable
static void run_command_and_set_env(const char *env_name, const char *command[]) {
    // Only Run If Environmental Variable Is NULL
    if (getenv(env_name) == nullptr) {
        // Check $DISPLAY
        reborn_check_display();
        // Run
        int return_code;
        char *output = run_command(command, &return_code, nullptr);
        if (output != nullptr) {
            // Trim
            const size_t length = strlen(output);
            if (output[length - 1] == '\n') {
                output[length - 1] = '\0';
            }
            // Set
            set_and_print_env(env_name, output);
            // Free
            free(output);
        }
        // Check Return Code
        if (!is_exit_status_success(return_code)) {
            // Launch Interrupted
            exit(EXIT_SUCCESS);
        }
    }
}

// Use Zenity To Set Environmental Variable
#define DIALOG_TITLE "Launcher"
static void run_zenity_and_set_env(const char *env_name, std::vector<std::string> command) {
    // Create Full Command
    std::vector<std::string> full_command;
    full_command.push_back("zenity");
    full_command.push_back("--title");
    full_command.push_back(DIALOG_TITLE);
    full_command.push_back("--name");
    full_command.push_back(MCPI_APP_ID);
    full_command.insert(full_command.end(), command.begin(), command.end());
    // Convert To C Array
    const char *full_command_array[full_command.size() + 1];
    for (std::vector<std::string>::size_type i = 0; i < full_command.size(); i++) {
        full_command_array[i] = full_command[i].c_str();
    }
    full_command_array[full_command.size()] = nullptr;
    // Run
    run_command_and_set_env(env_name, full_command_array);
}

// Set Variable If Not Already Set
static void set_env_if_unset(const char *env_name, const std::function<std::string()> &callback) {
    if (getenv(env_name) == nullptr) {
        char *value = strdup(callback().c_str());
        ALLOC_CHECK(value);
        set_and_print_env(env_name, value);
        free(value);
    }
}

// Handle Non-Launch Commands
void handle_non_launch_client_only_commands(const options_t &options) {
    // Print Available Feature Flags
    if (options.print_available_feature_flags) {
        load_available_feature_flags([](const std::string &line) {
            printf("%s\n", line.c_str());
            fflush(stdout);
        });
        exit(EXIT_SUCCESS);
    }
    // Wipe Cache If Needed
    if (options.wipe_cache) {
        wipe_cache();
        exit(EXIT_SUCCESS);
    }
}

// Configure Client Options
#define LIST_DIALOG_SIZE "400"
void configure_client(const options_t &options) {
    // Load Cache
    launcher_cache cache = options.no_cache ? empty_cache : load_cache();

    // --default
    if (options.use_default) {
        // Use Default Feature Flags
        set_env_if_unset(MCPI_FEATURE_FLAGS_ENV, [&cache]() {
            std::string feature_flags = "";
            load_available_feature_flags([&feature_flags, &cache](const std::string &flag) {
                bool value;
                // Strip Default Value
                std::string stripped_flag = strip_feature_flag_default(flag, &value);
                // Use Cache
                if (cache.feature_flags.count(stripped_flag) > 0) {
                    value = cache.feature_flags[stripped_flag];
                }
                // Specify Default Value
                if (value) {
                    // Enabled By Default
                    feature_flags += stripped_flag + '|';
                }
            });
            if (!feature_flags.empty() && feature_flags[feature_flags.length() - 1] == '|') {
                feature_flags.pop_back();
            }
            return feature_flags;
        });
        set_env_if_unset(MCPI_RENDER_DISTANCE_ENV, [&cache]() {
            return cache.render_distance;
        });
        set_env_if_unset(MCPI_USERNAME_ENV, [&cache]() {
            return cache.username;
        });
    }

    // Setup MCPI_FEATURE_FLAGS
    {
        std::vector<std::string> command;
        command.push_back("--list");
        command.push_back("--checklist");
        command.push_back("--width");
        command.push_back(LIST_DIALOG_SIZE);
        command.push_back("--height");
        command.push_back(LIST_DIALOG_SIZE);
        command.push_back("--column");
        command.push_back("Enabled");
        command.push_back("--column");
        command.push_back("Feature");
        load_available_feature_flags([&command, &cache](const std::string &flag) {
            bool value;
            // Strip Default Value
            std::string stripped_flag = strip_feature_flag_default(flag, &value);
            // Use Cache
            if (cache.feature_flags.count(stripped_flag) > 0) {
                value = cache.feature_flags[stripped_flag];
            }
            // Specify Default Value
            if (value) {
                // Enabled By Default
                command.push_back("TRUE");
            } else {
                // Disabled By Default
                command.push_back("FALSE");
            }
            // Specify Name
            command.push_back(stripped_flag);
        });
        // Run
        run_zenity_and_set_env(MCPI_FEATURE_FLAGS_ENV, command);
    }
    // Setup MCPI_RENDER_DISTANCE
    {
        std::vector<std::string> command;
        command.push_back("--list");
        command.push_back("--radiolist");
        command.push_back("--width");
        command.push_back(LIST_DIALOG_SIZE);
        command.push_back("--height");
        command.push_back(LIST_DIALOG_SIZE);
        command.push_back("--text");
        command.push_back("Select Minecraft Render Distance:");
        command.push_back("--column");
        command.push_back("Selected");
        command.push_back("--column");
        command.push_back("Name");
        std::string render_distances[] = {"Far", "Normal", "Short", "Tiny"};
        for (std::string &render_distance : render_distances) {
            command.push_back(render_distance == cache.render_distance ? "TRUE" : "FALSE");
            command.push_back(render_distance);
        }
        // Run
        run_zenity_and_set_env(MCPI_RENDER_DISTANCE_ENV, command);
    }
    // Setup MCPI_USERNAME
    {
        std::vector<std::string> command;
        command.push_back("--entry");
        command.push_back("--text");
        command.push_back("Enter Minecraft Username:");
        command.push_back("--entry-text");
        command.push_back(cache.username);
        // Run
        run_zenity_and_set_env(MCPI_USERNAME_ENV, command);
    }

    // Save Cache
    if (!options.no_cache) {
        save_cache();
    }
}
