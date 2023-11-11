// Config Needs To Load First
#include <libreborn/libreborn.h>

// Game Mode UI Code Is Useless In Headless Mode
#ifndef MCPI_SERVER_MODE

#include <pthread.h>
#include <cstring>
#include <ctime>
#include <string>
#include <stdexcept>

#include <symbols/minecraft.h>
#include <media-layer/core.h>

#include "game-mode-internal.h"

// Run Command
static char *run_command_proper(const char *command[], bool allow_empty) {
    // Run
    int return_code;
    char *output = run_command(command, &return_code, NULL);

    // Handle Message
    if (output != NULL) {
        // Check Return Code
        if (is_exit_status_success(return_code)) {
            // Remove Ending Newline
            int length = strlen(output);
            if (output[length - 1] == '\n') {
                output[length - 1] = '\0';
            }
            length = strlen(output);
            // Don't Allow Empty Strings
            if (allow_empty || length > 0) {
                // Return
                return output;
            }
        }
        // Free Output
        free(output);
    }
    // Return
    return !is_exit_status_success(return_code) ? NULL : run_command_proper(command, allow_empty);
}

// Track Create World State
static pthread_mutex_t create_world_state_lock = PTHREAD_MUTEX_INITIALIZER;
typedef enum {
    DIALOG_CLOSED,
    DIALOG_OPEN,
    DIALOG_SUCCESS
} create_world_state_dialog_t;
typedef struct {
    volatile create_world_state_dialog_t dialog_state = DIALOG_CLOSED;
    volatile char *name = NULL;
    volatile int32_t game_mode = 0;
    volatile int32_t seed = 0;
} create_world_state_t;
static create_world_state_t create_world_state;
// Destructor
__attribute__((destructor)) static void _free_create_world_state_name() {
    free((void *) create_world_state.name);
}

// Reset State (Assume Lock)
static void reset_create_world_state() {
    create_world_state.dialog_state = DIALOG_CLOSED;
    if (create_world_state.name != NULL) {
        free((void *) create_world_state.name);
    }
    create_world_state.name = NULL;
    create_world_state.game_mode = 0;
    create_world_state.seed = 0;
}

// Chat Thread
#define DEFAULT_WORLD_NAME "Unnamed world"
#define DIALOG_TITLE "Create World"
#define GAME_MODE_DIALOG_SIZE "200"
static void *create_world_thread(__attribute__((unused)) void *nop) {
    // Run Dialogs
    {
        // World Name
        char *world_name = NULL;
        {
            // Open
            const char *command[] = {
                "zenity",
                "--title", DIALOG_TITLE,
                "--name", MCPI_APP_ID,
                "--entry",
                "--text", "Enter World Name:",
                "--entry-text", DEFAULT_WORLD_NAME,
                NULL
            };
            char *output = run_command_proper(command, false);
            // Handle Message
            if (output != NULL) {
                // Store
                world_name = strdup(output);
                ALLOC_CHECK(world_name);
                // Free
                free(output);
            } else {
                // Fail
                goto fail;
            }
        }

        // Game Mode
        int game_mode = 0;
        {
            // Open
            const char *command[] = {
                "zenity",
                "--title", DIALOG_TITLE,
                "--name", MCPI_APP_ID,
                "--list",
                "--radiolist",
                "--width", GAME_MODE_DIALOG_SIZE,
                "--height", GAME_MODE_DIALOG_SIZE,
                "--text", "Select Game Mode:",
                "--column","Selected",
                "--column", "Name",
                "TRUE", "Creative",
                "FALSE", "Survival",
                NULL
            };
            char *output = run_command_proper(command, false);
            // Handle Message
            if (output != NULL) {
                // Store
                game_mode = strcmp(output, "Creative") == 0;
                // Free
                free(output);
            } else {
                // Fail
                goto fail;
            }
        }

        // Seed
        int32_t seed = 0;
 get_seed:
        {
            // Open
            const char *command[] = {
                "zenity",
                "--title", DIALOG_TITLE,
                "--name", MCPI_APP_ID,
                "--entry",
                "--only-numerical",
                "--text", "Enter Seed (Leave Blank For Random):",
                NULL
            };
            char *output = run_command_proper(command, true);
            // Handle Message
            if (output != NULL) {
                // Store
                bool valid = true;
                try {
                    seed = strlen(output) == 0 ? time(NULL) : std::stoi(output);
                } catch (std::invalid_argument &e) {
                    // Invalid Seed
                    WARN("Invalid Seed: %s", output);
                    valid = false;
                } catch (std::out_of_range &e) {
                    // Out-Of-Range Seed
                    WARN("Seed Out-Of-Range: %s", output);
                    valid = false;
                }
                // Free
                free(output);
                // Retry If Invalid
                if (!valid) {
                    goto get_seed;
                }
            } else {
                // Fail
                goto fail;
            }
        }

        // Update State
        pthread_mutex_lock(&create_world_state_lock);
        reset_create_world_state();
        create_world_state.dialog_state = DIALOG_SUCCESS;
        char *safe_name = to_cp437(world_name);
        create_world_state.name = safe_name;
        free(world_name);
        create_world_state.game_mode = game_mode;
        create_world_state.seed = seed;
        pthread_mutex_unlock(&create_world_state_lock);
        // Return
        return NULL;
    }

 fail:
    // Update State
    pthread_mutex_lock(&create_world_state_lock);
    reset_create_world_state();
    pthread_mutex_unlock(&create_world_state_lock);
    // Return
    return NULL;
}

// Create Chat Thead
static void open_create_world() {
    // Update State (Assume Lock)
    create_world_state.dialog_state = DIALOG_OPEN;
    // Start Thread
    pthread_t thread;
    pthread_create(&thread, NULL, create_world_thread, NULL);
}

// Get Minecraft From Screen
static unsigned char *get_minecraft_from_screen(unsigned char *screen) {
    return *(unsigned char **) (screen + Screen_minecraft_property_offset);
}

// Create World
static void create_world(unsigned char *host_screen, std::string folder_name) {
    // Get Minecraft
    unsigned char *minecraft = get_minecraft_from_screen(host_screen);

    // Settings
    LevelSettings settings;
    settings.game_type = create_world_state.game_mode;
    settings.seed = create_world_state.seed;

    // Create World
    std::string world_name = (char *) create_world_state.name;
    (*Minecraft_selectLevel)(minecraft, folder_name, world_name, settings);

    // Multiplayer
    (*Minecraft_hostMultiplayer)(minecraft, 19132);

    // Open ProgressScreen
    unsigned char *screen = (unsigned char *) ::operator new(PROGRESS_SCREEN_SIZE);
    ALLOC_CHECK(screen);
    screen = (*ProgressScreen)(screen);
    (*Minecraft_setScreen)(minecraft, screen);

    // Reset
    reset_create_world_state();
}

// Redirect Create World Button
#define create_SelectWorldScreen_tick_injection(prefix) \
    static void prefix##SelectWorldScreen_tick_injection(unsigned char *screen) { \
        /* Lock */ \
        pthread_mutex_lock(&create_world_state_lock); \
        \
        bool *should_create_world = (bool *) (screen + prefix##SelectWorldScreen_should_create_world_property_offset); \
        if (*should_create_world) { \
            /* Check State */ \
            if (create_world_state.dialog_state == DIALOG_CLOSED) { \
                /* Open Dialog */ \
                open_create_world(); \
            } \
            \
            /* Finish */ \
            *should_create_world = false; \
        } else { \
            /* Call Original Method */ \
            (*prefix##SelectWorldScreen_tick)(screen); \
        } \
        \
        /* Create World If Dialog Succeeded */ \
        if (create_world_state.dialog_state == DIALOG_SUCCESS) { \
            /* Create World Dialog Finished */ \
            \
            /* Get New World Name */ \
            std::string name = (char *) create_world_state.name; \
            std::string new_name = (*prefix##SelectWorldScreen_getUniqueLevelName)(screen, name); \
            \
            /* Create World */ \
            create_world(screen, new_name); \
        } \
        \
        /* Lock/Unlock UI */ \
        if (create_world_state.dialog_state != DIALOG_OPEN) { \
            /* Dialog Closed, Unlock UI */ \
            media_set_interactable(1); \
        } else { \
            /* Dialog Open, Lock UI */ \
            media_set_interactable(0); \
        } \
        \
        /* Unlock */ \
        pthread_mutex_unlock(&create_world_state_lock); \
    }
create_SelectWorldScreen_tick_injection()
create_SelectWorldScreen_tick_injection(Touch_)

// Init
void _init_game_mode_ui() {
    // Hijack Create World Button
    patch_address(SelectWorldScreen_tick_vtable_addr, (void *) SelectWorldScreen_tick_injection);
    patch_address(Touch_SelectWorldScreen_tick_vtable_addr, (void *) Touch_SelectWorldScreen_tick_injection);
}

#else
void _init_game_mode_ui() {
}
#endif
