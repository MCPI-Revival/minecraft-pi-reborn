#include <fstream>
#include <cstring>
#include <cerrno>
#include <sys/wait.h>
#include <sys/stat.h>
#include <vector>
#include <functional>

#include <libreborn/libreborn.h>

#include "../bootstrap.h"

// Load Available Feature Flags
static void load_available_feature_flags(std::function<void(std::string)> callback) {
    // Get Path
    char *binary_directory = get_binary_directory();
    std::string path = std::string(binary_directory) + "/available-feature-flags";
    free(binary_directory);
    // Load File
    std::ifstream stream(path);
    if (stream && stream.good()) {
        std::string line;
        while (std::getline(stream, line)) {
            if (line.length() > 0) {
                // Verify Line
                if (line.find('|') == std::string::npos) {
                    callback(line);
                } else {
                    // Invalid Line
                    ERR("%s", "Feature Flag Contains Invalid '|'");
                }
            }
        }
        stream.close();
    } else {
        ERR("%s", "Unable To Load Available Feature Flags");
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
        if (return_code != 0) {
            ERR("%s", "Launch Interrupted");
        }
    }
}

// Use Zenity To Set Environmental Variable
static void run_zenity_and_set_env(const char *env_name, std::vector<std::string> command) {
    // Create Full Command
    std::vector<std::string> full_command;
    full_command.push_back("zenity");
    full_command.push_back("--class");
    full_command.push_back(GUI_TITLE);
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

// Launch
int main(int argc, char *argv[]) {
    // Pre-Bootstrap
    pre_bootstrap();

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

    // Create ~/.minecraft-pi If Needed
    // Minecraft Folder
    {
        char *minecraft_folder = NULL;
        safe_asprintf(&minecraft_folder, "%s/.minecraft-pi", getenv("HOME"));
        {
            // Check Minecraft Folder
            struct stat obj;
            if (stat(minecraft_folder, &obj) != 0 || !S_ISDIR(obj.st_mode)) {
                // Create Minecraft Folder
                int ret = mkdir(minecraft_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                if (ret != 0) {
                    // Unable To Create Folder
                    ERR("Error Creating Directory: %s: %s", minecraft_folder, strerror(errno));
                }
            }
        }
        free(minecraft_folder);
    }

    // Setup MCPI_FEATURE_FLAGS
    {
        std::vector<std::string> command;
        command.push_back("--list");
        command.push_back("--checklist");
        command.push_back("--width");
        command.push_back("400");
        command.push_back("--height");
        command.push_back("400");
        command.push_back("--column");
        command.push_back("Enabled");
        command.push_back("--column");
        command.push_back("Feature");
        load_available_feature_flags([&command](std::string line) {
            if (line.rfind("TRUE ", 0) == 0) {
                // Enabled By Default
                command.push_back("TRUE");
                command.push_back(line.substr(5, std::string::npos));
            } else if (line.rfind("FALSE ", 0) == 0) {
                // Disabled By Default
                command.push_back("FALSE");
                command.push_back(line.substr(6, std::string::npos));
            } else {
                ERR("%s", "Invalid Feature Flag Default");
            }
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
        command.push_back("400");
        command.push_back("--height");
        command.push_back("400");
        command.push_back("--text");
        command.push_back("Minecraft Render Distance:");
        command.push_back("--column");
        command.push_back("Selected");
        command.push_back("--column");
        command.push_back("Name");
        command.push_back("FALSE");
        command.push_back("Far");
        command.push_back("FALSE");
        command.push_back("Normal");
        command.push_back("TRUE");
        command.push_back("Short");
        command.push_back("FALSE");
        command.push_back("Tiny");
        // Run
        run_zenity_and_set_env("MCPI_RENDER_DISTANCE", command);
    }
    // Setup MCPI_USERNAME
    {
        std::vector<std::string> command;
        command.push_back("--entry");
        command.push_back("--text");
        command.push_back("Minecraft Username:");
        command.push_back("--entry-text");
        command.push_back("StevePi");
        // Run
        run_zenity_and_set_env("MCPI_USERNAME", command);
    }

    // Bootstrap
    bootstrap(argc, argv);
}
