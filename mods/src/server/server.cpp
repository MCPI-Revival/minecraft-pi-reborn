#include <string>
#include <stdint.h>
#include <ctime>
#include <cstdio>
#include <csignal>
#include <fstream>

#include <unistd.h>

#include <SDL/SDL_events.h>

#include <libcore/libcore.h>

#include "server.h"
#include "server_properties.h"

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

#define INFO(msg, ...) fprintf(stderr, "[INFO]: " msg "\n", __VA_ARGS__);

// Store Minecraft For Exit
static unsigned char *stored_minecraft = NULL;

// Server Properties
static ServerProperties &get_server_properties() {
    static ServerProperties properties;
    return properties;
}

// Default Server Properties
#define DEFAULT_MOTD "Minecraft Server"
#define DEFAULT_GAME_MODE "0"
#define DEFAULT_PORT "19132"
#define DEFAULT_SEED ""
#define DEFAULT_MOB_SPAWNING "true"
#define DEFAULT_WORLD_NAME "world"

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

// Runs Every Tick
static int last_progress = -1;
static const char *last_message = NULL;
static bool loaded = false;
static void Minecraft_update_injection(unsigned char *minecraft) {
    // Create/Start World
    if (!loaded) {
        INFO("%s", "Starting Minecraft: Pi Edition Dedicated Server");

        LevelSettings settings;
        settings.game_type = 0; // Patched By MCPI-Docker
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

        stored_minecraft = minecraft;

        loaded = true;
    }
    
    // Print Progress Message
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

    // Call Original Method
    revert_overwrite((void *) Minecraft_update, Minecraft_update_original);
    (*Minecraft_update)(minecraft);
    revert_overwrite((void *) Minecraft_update, Minecraft_update_original);
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

static void exit_handler(__attribute__((unused)) int data) {
    INFO("%s", "Stopping Server");
    if (stored_minecraft != NULL) {
        unsigned char *level = *(unsigned char **) (stored_minecraft + 0x188);
        if (level != NULL) {
            // Save Game
            (*Level_saveLevelData)(level);
        }
    }
    // Stop Game
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

const char *server_get_motd() {
    return get_server_properties().get_string("motd", DEFAULT_MOTD).c_str();
}
int server_get_default_game_mode() {
    return get_server_properties().get_int("game-mode", DEFAULT_GAME_MODE);
}
int server_get_mob_spawning() {
    return get_server_properties().get_bool("spawn-mobs", DEFAULT_MOB_SPAWNING);
}

void server_init() {
    // Open Properties File
    std::string file(getenv("HOME"));
    file.append("/.minecraft/server.properties");

    std::ifstream properties_file(file);

    if (!properties_file || !properties_file.is_open()) {
        // Write Defaults
        std::ofstream properties_file_output(file);
        properties_file_output << "motd=" DEFAULT_MOTD "\n";
        properties_file_output << "game-mode=" DEFAULT_GAME_MODE "\n";
        properties_file_output << "port=" DEFAULT_PORT "\n";
        properties_file_output << "seed=" DEFAULT_SEED "\n";
        properties_file_output << "spawn-mobs=" DEFAULT_MOB_SPAWNING "\n";
        properties_file_output << "world-name=" DEFAULT_WORLD_NAME "\n";
        properties_file_output.close();
        // Re-Open File
        properties_file = std::ifstream(file);
    }

    if (!properties_file.is_open()) {
        fprintf(stderr, "[ERR]: Unable To Open server.properties\n");
        exit(1);
    }

    // Load Properties
    get_server_properties().load(properties_file);

    properties_file.close();

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
}
