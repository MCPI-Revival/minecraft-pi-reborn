#include "util.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <libreborn/log.h>
#include <libreborn/env/env.h>

// Get Binary Location
std::string get_binary() {
#ifdef _WIN32
    constexpr int binary_directory_max_length = MAX_PATH;
    char binary_directory_raw[binary_directory_max_length] = {};
    const bool ret = GetModuleFileNameA(nullptr, binary_directory_raw, binary_directory_max_length);
    if (!ret) {
        ERR("Unable To Locate Binary");
    }
    return binary_directory_raw;
#else
    return safe_realpath("/proc/self/exe");
#endif
}

// Get Directory Containing Binary
std::string get_binary_directory() {
    std::string exe = get_binary();
    chop_last_component(exe);
    return exe;
}

// Get AppImage Path
std::string get_appimage_path() {
    return require_env("APPIMAGE");
}