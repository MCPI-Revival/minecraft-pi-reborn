#pragma once

#include <cstdint>
#include <string>

// Hook Library Function
#ifndef _WIN32
#include <dlfcn.h>
#include "../log.h"
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
#endif

// Align Number
MCPI_REBORN_UTIL_PUBLIC int align_up(int x, int alignment);

// Check If Two Percentages Are Different Enough To Be Logged
MCPI_REBORN_UTIL_PUBLIC bool is_progress_difference_significant(int32_t new_val, int32_t old_val);

// Check $DISPLAY
MCPI_REBORN_UTIL_PUBLIC void reborn_check_display();

// Get Home Subdirectory
MCPI_REBORN_UTIL_PUBLIC const char *get_home_subdirectory_for_game_data();

// Make Sure Directory Exists
MCPI_REBORN_UTIL_PUBLIC void ensure_directory(const char *path);

// embed_resource()
#define EMBEDDED_RESOURCE(name, public_macro) \
    public_macro extern unsigned char (name)[]; \
    public_macro extern size_t name##_len

// Profile Directory
MCPI_REBORN_UTIL_PUBLIC std::string home_get();

// Default MCPI Port
// This Macro DOES NOT Control MCPI
#define DEFAULT_MULTIPLAYER_PORT 19132

// Locate The "Main" Executable
// Used For Locating Resources And Relaunching
#ifdef _WIN32
MCPI_REBORN_UTIL_PUBLIC std::wstring get_launcher_executable();
MCPI_REBORN_UTIL_PUBLIC std::pair<std::wstring, int> get_display_name_resource();
#endif