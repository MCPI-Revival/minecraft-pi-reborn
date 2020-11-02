#include <stdint.h>
#include <stdio.h>

#include <libcore/libcore.h>

#include "extra.h"
#include "server/server.h"

static int mob_spawning = 0;
static uint32_t LevelData_getSpawnMobs_injection(__attribute__((unused)) int32_t obj) {
    return mob_spawning;
}

static int is_right_click = 0;
void extra_set_is_right_click(int val) {
    is_right_click = val;
}

typedef void (*releaseUsingItem_t)(unsigned char *game_mode, unsigned char *player);

typedef void (*Minecraft_tickInput_t)(unsigned char *minecraft, uint32_t param_1);
static Minecraft_tickInput_t Minecraft_tickInput = (Minecraft_tickInput_t) 0x15ffc;
static void *Minecraft_tickInput_original = NULL;

typedef int (*Player_isUsingItem_t)(unsigned char *player);
static Player_isUsingItem_t Player_isUsingItem = (Player_isUsingItem_t) 0x8f15c;

static void Minecraft_tickInput_injection(unsigned char *minecraft, uint32_t param_1) {
    // Call Original Method
    revert_overwrite((void *) Minecraft_tickInput, Minecraft_tickInput_original);
    (*Minecraft_tickInput)(minecraft, param_1);
    revert_overwrite((void *) Minecraft_tickInput, Minecraft_tickInput_original);

    // GameMode Is Offset From param_1 By 0x160
    // Player Is Offset From param_1 By 0x18c
    if (!is_right_click) {
        unsigned char *game_mode = *(unsigned char **) (minecraft + 0x160);
        unsigned char *player = *(unsigned char **) (minecraft + 0x18c);
        if (player != NULL && game_mode != NULL && (*Player_isUsingItem)(player)) {
            unsigned char *game_mode_vtable = *(unsigned char **) game_mode;
            releaseUsingItem_t releaseUsingItem = *(releaseUsingItem_t *) (game_mode_vtable + 0x5c);
            (*releaseUsingItem)(game_mode, player);
        }
    }

    extra_clear_input();
}

typedef void (*Gui_tickItemDrop_t)(unsigned char *);
static Gui_tickItemDrop_t Gui_tickItemDrop = (Gui_tickItemDrop_t) 0x27778;
static void *Gui_tickItemDrop_original = NULL;

#include <SDL/SDL_events.h>

static void Gui_tickItemDrop_injection(unsigned char *this) {
    if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) {
        revert_overwrite((void *) Gui_tickItemDrop, Gui_tickItemDrop_original);
        (*Gui_tickItemDrop)(this);
        revert_overwrite((void *) Gui_tickItemDrop, Gui_tickItemDrop_original);
    }
}

typedef void (*Gui_handleClick_t)(unsigned char *, unsigned char *, unsigned char *, unsigned char *);
static Gui_handleClick_t Gui_handleClick = (Gui_handleClick_t) 0x2599c;
static void *Gui_handleClick_original = NULL;

static void Gui_handleClick_injection(unsigned char *this, unsigned char *param_2, unsigned char *param_3, unsigned char *param_4) {
    if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE) {
        revert_overwrite((void *) Gui_handleClick, Gui_handleClick_original);
        (*Gui_handleClick)(this, param_2, param_3, param_4);
        revert_overwrite((void *) Gui_handleClick, Gui_handleClick_original);
    }
}

static int is_survival = -1;

static void *Creator = (void *) 0x1a044;
static void *SurvivalMode = (void *) 0x1b7d8;
static void *CreativeMode = (void *) 0x1b258;

// Patch Game Mode
static void set_is_survival(int new_is_survival) {
    if (is_survival != new_is_survival) {
        INFO("Setting Game Mode: %s", new_is_survival ? "Survival" : "Creative");

        // Correct Inventpry UI
        unsigned char inventory_patch[4] = {new_is_survival ? 0x00 : 0x01, 0x30, 0xa0, 0xe3};
        patch((void *) 0x16efc, inventory_patch);

        // Use Correct Size For GameMode Object
        unsigned char size_patch[4] = {new_is_survival ? 0x24 : 0x18, 0x00, 0xa0, 0xe3};
        patch((void *) 0x16ee4, size_patch);

        // Replace Creator Constructor With CreatorMode Or SurvivalMode Constructor
        overwrite(Creator, new_is_survival ? SurvivalMode : CreativeMode);

        is_survival = new_is_survival;
    }
}

typedef void (*Minecraft_setIsCreativeMode_t)(unsigned char *, int32_t);
static Minecraft_setIsCreativeMode_t Minecraft_setIsCreativeMode = (Minecraft_setIsCreativeMode_t) 0x16ec4;
static void *Minecraft_setIsCreativeMode_original = NULL;

static void Minecraft_setIsCreativeMode_injection(unsigned char *this, int32_t new_game_mode) {
    set_is_survival(!new_game_mode);

    revert_overwrite((void *) Minecraft_setIsCreativeMode, Minecraft_setIsCreativeMode_original);
    (*Minecraft_setIsCreativeMode)(this, new_game_mode);
    revert_overwrite((void *) Minecraft_setIsCreativeMode, Minecraft_setIsCreativeMode_original);
}

static char *get_username() {
    char *username = getenv("MCPI_USERNAME");
    if (username == NULL) {
        username = "StevePi";
    }
    return username;
}

typedef void (*Minecraft_init_t)(unsigned char *);
static Minecraft_init_t Minecraft_init = (Minecraft_init_t) 0x1700c;
static void *Minecraft_init_original = NULL;

static void Minecraft_init_injection(unsigned char *this) {
    revert_overwrite((void *) Minecraft_init, Minecraft_init_original);
    (*Minecraft_init)(this);
    revert_overwrite((void *) Minecraft_init, Minecraft_init_original);

    // Enable Fancy Graphics
    *(this + 83) = 1;
}

// Is Dedicated Server
static int is_server = 0;

// Check For Feature
int extra_has_feature(const char *name) {
    if (is_server) {
        // Enable All Features In Server
        return 1;
    } else {
        char *env = getenv("MCPI_FEATURES");
        char *features = strdup(env != NULL ? env : "");
        char *tok = strtok(features, "|");
        int ret = 0;
        while (tok != NULL) {
            if (strcmp(tok, name) == 0) {
                ret = 1;
                break;
            }
            tok = strtok(NULL, "|");
        }
        free(features);
        INFO("Feature: %s: %s", name, ret ? "Enabled" : "Disabled");
        return ret;
    }
}

int extra_get_mode() {
    char *mode = getenv("MCPI_MODE");
    if (mode == NULL) {
        ERR("%s", "MCPI Mode Not Specified");
    } else if (strcmp("virgl", mode) == 0) {
        return 0;
    } else if (strcmp("native", mode) == 0) {
        return 1;
    } else if (strcmp("server", mode) == 0) {
        return 2;
    } else {
        ERR("Inavlid MCPI_MODE: %s", mode);
    }
}

__attribute__((constructor)) static void init() {
    is_server = extra_get_mode() == 2;
    if (is_server) {
        server_init();
    }

    if (extra_has_feature("Touch GUI")) {
        // Use Touch UI
        unsigned char touch_gui_patch[4] = {0x01, 0x00, 0x50, 0xe3};
        patch((void *) 0x292fc, touch_gui_patch);
    }

    // Dyanmic Game Mode Switching
    set_is_survival(1);
    Minecraft_setIsCreativeMode_original = overwrite((void *) Minecraft_setIsCreativeMode, Minecraft_setIsCreativeMode_injection);

    // Get Default Game Mode
    if (!is_server) {
        int default_game_mode = !extra_has_feature("Survival Mode");

        // Set Default Game Mode
        unsigned char default_game_mode_patch[4] = {default_game_mode ? 0x01 : 0x00, 0x30, 0xa0, 0xe3};
        patch((void *) 0x3d9b8, default_game_mode_patch);
        patch((void *) 0x38a78, default_game_mode_patch);
    }

    // Disable Item Dropping Using The Cursor When Cursor Is Hidden
    Gui_tickItemDrop_original = overwrite((void *) Gui_tickItemDrop, Gui_tickItemDrop_injection);
    // Disable Opening Inventory Using The Cursor When Cursor Is Hidden
    Gui_handleClick_original = overwrite((void *) Gui_handleClick, Gui_handleClick_injection);

    if (extra_has_feature("Fix Bow & Arrow")) {
        // Fix Bow
        Minecraft_tickInput_original = overwrite((void *) Minecraft_tickInput, Minecraft_tickInput_injection);
    }

    if (extra_has_feature("Fix Attacking")) {
        // Allow Attacking Mobs
        unsigned char attacking_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
        patch((void *) 0x162d4, attacking_patch);
        // Fix Instamining When Using This Patch
        unsigned char instamine_patch[4] = {0x61, 0x00, 0x00, 0xea};
        patch((void *) 0x15b0c, instamine_patch);
        // Fix Excessive Hand Swinging When Using This Patch
        unsigned char excessive_swing_patch[4] = {0x06, 0x00, 0x00, 0xea};
        patch((void *) 0x1593c, excessive_swing_patch);
    }

    if (is_server) {
        mob_spawning = server_get_mob_spawning();
    } else {
        mob_spawning = extra_has_feature("Mob Spawning");
    }
    // Set Mob Spawning
    overwrite((void *) 0xbabec, LevelData_getSpawnMobs_injection);

    // Replace CreatorLevel With ServerLevel (This Fixes Beds And Mob Spawning)
    unsigned char patch_data_4[4] = {0x68, 0x7e, 0x01, 0xeb};
    patch((void *) 0x16f84, patch_data_4);

    // Allocate Correct Size For ServerLevel
    unsigned char patch_data_5[4] = {0x94, 0x0b, 0x00, 0x00};
    patch((void *) 0x17004, patch_data_5);

    if (extra_has_feature("Fancy Graphics")) {
        // Enable Fancy Graphics
        Minecraft_init_original = overwrite((void *) Minecraft_init, Minecraft_init_injection);
    }

    // Allow Connecting To Non-Pi Servers
    unsigned char patch_data_9[4] = {0x0f, 0x00, 0x00, 0xea};
    patch((void *) 0x6dc70, patch_data_9);

    // Change Username
    const char *username;
    if (is_server) {
        // MOTD is Username
        username = server_get_motd();
    } else {
        username = get_username();
        INFO("Setting Username: %s", username);
    }
    char **default_username = (char **) 0x18fd4;
    if (strcmp(*default_username, "StevePi") != 0) {
        ERR("%s", "Default Username Is Invalid");
    }
    patch_address((void *) default_username, (void *) username);

    if (extra_has_feature("Disable Autojump By Default")) {
        // Disable Autojump By Default
        unsigned char autojump_patch[4] = {0x00, 0x30, 0xa0, 0xe3};
        patch((void *) 0x44b90, autojump_patch);
    }

    if (extra_has_feature("Show Block Outlines")) {
        // Show Block Outlines
        unsigned char outline_patch[4] = {0x00, 0xf0, 0x20, 0xe3};
        patch((void *) 0x4a214, outline_patch);
    }
}
