#pragma once

#include <dlfcn.h>
#include <string>

#include "../log.h"

// Align Number
int align_up(int x, int alignment);

// Hook Library Function
#define HOOK(name, return_type, args) \
    typedef return_type (*real_##name##_t)args; \
    MCPI_UNUSED static real_##name##_t real_##name() { \
        static real_##name##_t func = NULL; \
        if (!func) { \
            dlerror(); \
            func = (real_##name##_t) dlsym(RTLD_NEXT, #name); \
            if (!func) { \
                ERR("Error Resolving Symbol: " #name ": %s", dlerror()); \
            } \
        } \
        return func; \
    } \
    extern "C" __attribute__((used, visibility("default"))) return_type name args

// Check If Two Percentages Are Different Enough To Be Logged
bool is_progress_difference_significant(int32_t new_val, int32_t old_val);

// Check $DISPLAY
void reborn_check_display();

// Get Home Subdirectory
const char *get_home_subdirectory_for_game_data();

// Make Sure Directory Exists
void ensure_directory(const char *path);

// embed_resource()
#define EMBEDDED_RESOURCE(name) \
    extern unsigned char name[]; \
    extern size_t name##_len

// Profile Directory
std::string home_get();

// Default MCPI Port
// This Macro DOES NOT Control MCPI
#define DEFAULT_MULTIPLAYER_PORT 19132