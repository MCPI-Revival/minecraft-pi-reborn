#include <sstream>
#include <cstring>
#include <cerrno>
#include <sys/wait.h>
#include <sys/stat.h>
#include <vector>
#include <functional>
#include <algorithm>

#include <libreborn/libreborn.h>

#include "../bootstrap.h"
#include "launcher.h"
#include "cache.h"

// Strip Feature Flag Default
std::string strip_feature_flag_default(std::string flag, bool *default_ret) {
    // Valid Values
    std::string true_str = "TRUE ";
    std::string false_str = "FALSE ";
    // Test
    if (flag.rfind(true_str, 0) == 0) {
        // Enabled By Default
        if (default_ret != NULL) {
            *default_ret = true;
        }
        return flag.substr(true_str.length(), std::string::npos);
    } else if (flag.rfind(false_str, 0) == 0) {
        // Disabled By Default
        if (default_ret != NULL) {
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
void load_available_feature_flags(std::function<void(std::string)> callback) {
    // Get Path
    char *binary_directory = get_binary_directory();
    std::string path = std::string(binary_directory) + "/available-feature-flags";
    free(binary_directory);
    // Load File
    std::string data(available_feature_flags, available_feature_flags + available_feature_flags_len);
    std::stringstream stream(data);
    // Store Lines
    std::vector<std::string> lines;
    // Read File
    {
        std::string line;
        while (std::getline(stream, line)) {
            if (line.length() > 0) {
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
    std::sort(lines.begin(), lines.end(), [](std::string a, std::string b) {
        // Strip Defaults
        std::string stripped_a = strip_feature_flag_default(a, NULL);
        std::string stripped_b = strip_feature_flag_default(b, NULL);
        // Sort
        return stripped_a < stripped_b;
    });
    // Run Callbacks
    for (std::string &line : lines) {
        callback(line);
    }
}

// Run Command And Set Environmental Variable
static void run_command_and_set_env(const char *env_name, const char *command[]) {
    // Only Run If Environmental Variable Is NULL
    if (getenv(env_name) == NULL) {
        // Run
        int return_code;
        char *output = run_command(command, &return_code);
        if (output != NULL) {
            // Trim
            int length = strlen(output);
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
            INFO("Launch Interrupted");
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
    full_command_array[full_command.size()] = NULL;
    // Run
    run_command_and_set_env(env_name, full_command_array);
}

// Set Variable If Not Already Set
static void set_env_if_unset(const char *env_name, std::function<std::string()> callback) {
    if (getenv(env_name) == NULL) {
        char *value = strdup(callback().c_str());
        ALLOC_CHECK(value);
        set_and_print_env(env_name, value);
        free(value);
    }
}

// Launch
#define LIST_DIALOG_SIZE "400"
int main(int argc, char *argv[]) {
    // Don't Run As Root
    if (getenv("_MCPI_SKIP_ROOT_CHECK") == NULL && (getuid() == 0 || geteuid() == 0)) {
        ERR("Don't Run As Root");
    }

    // Ensure HOME
    if (getenv("HOME") == NULL) {
        ERR("$HOME Isn't Set");
    }

    // Print Features
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-available-feature-flags") == 0) {
            // Print Available Feature Flags
            load_available_feature_flags([](std::string line) {
                printf("%s\n", line.c_str());
                fflush(stdout);
            });
            return 0;
        }
    }

    // Pre-Bootstrap
    pre_bootstrap(argc, argv);

    // Create ~/.minecraft-pi If Needed
    {
        char *minecraft_folder = NULL;
        safe_asprintf(&minecraft_folder, "%s" HOME_SUBDIRECTORY_FOR_GAME_DATA, getenv("HOME"));
        const char *const command[] = {"mkdir", "-p", minecraft_folder, NULL};
        run_simple_command(command, "Unable To Create Data Directory");
        free(minecraft_folder);
    }

    // --wipe-cache
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--wipe-cache") == 0) {
            wipe_cache();
            break;
        }
    }

    // --no-cache
    bool no_cache = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-cache") == 0) {
            no_cache = true;
            break;
        }
    }
    // Load Cache
    launcher_cache cache = no_cache ? empty_cache : load_cache();

    // --default
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--default") == 0) {
            // Use Default Feature Flags
            set_env_if_unset("MCPI_FEATURE_FLAGS", [&cache]() {
                std::string feature_flags = "";
                load_available_feature_flags([&feature_flags, &cache](std::string flag) {
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
                if (feature_flags.length() > 0 && feature_flags[feature_flags.length() - 1] == '|') {
                    feature_flags.pop_back();
                }
                return feature_flags;
            });
            set_env_if_unset("MCPI_RENDER_DISTANCE", [&cache]() {
                return cache.render_distance;
            });
            set_env_if_unset("MCPI_USERNAME", [&cache]() {
                return cache.username;
            });
            break;
        }
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
        load_available_feature_flags([&command, &cache](std::string flag) {
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
        run_zenity_and_set_env("MCPI_FEATURE_FLAGS", command);
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
            command.push_back(render_distance.compare(cache.render_distance) == 0 ? "TRUE" : "FALSE");
            command.push_back(render_distance);
        }
        // Run
        run_zenity_and_set_env("MCPI_RENDER_DISTANCE", command);
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
        run_zenity_and_set_env("MCPI_USERNAME", command);
    }

    // Save Cache
    if (!no_cache) {
        save_cache();
    }

    // Bootstrap
    bootstrap(argc, argv);
}
