#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

#include <unistd.h>
#include <fcntl.h>

#include <libreborn/libreborn.h>

#include <mods/override/override.h>
#include <mods/home/home.h>
#include <mods/init/init.h>

// Hook access
HOOK(access, int, (const char *pathname, int mode)) {
    char *new_path = override_get_path(pathname);
    // Open File
    ensure_access();
    const int ret = real_access(new_path != nullptr ? new_path : pathname, mode);
    // Free Data
    if (new_path != nullptr) {
        free(new_path);
    }
    // Return
    return ret;
}

// Get Override Folder
static std::string get_override_directory() {
    const std::string home_path = home_get();
    return home_path + "/overrides";
}

// Get Override Path For File (If It Exists)
char *override_get_path(const char *filename) {
    // Custom Skin
    if (starts_with(filename, "data/images/$")) {
        // Fallback Texture
        filename = "data/images/mob/char.png";
    }

    // Get Asset Override Path
    const std::string overrides = get_override_directory();

    // Data Prefiix
    const std::string data_prefix = "data/";
    int data_prefix_length = data_prefix.length();

    // Folders To Check
    std::string asset_folders[] = {
        overrides,
        getenv(_MCPI_REBORN_ASSETS_PATH_ENV),
        getenv(_MCPI_VANILLA_ASSETS_PATH_ENV),
        ""
    };

    // Check For Override
    std::string new_path;
    if (std::string(filename).rfind(data_prefix, 0) == 0) {
        // Test Asset Folders
        for (int i = 0; !asset_folders[i].empty(); i++) {
            new_path = asset_folders[i] + '/' + &filename[data_prefix_length];
            ensure_access();
            if (real_access(new_path.c_str(), F_OK) == -1) {
                // Not Found In Asset Folder
                new_path = "";
                continue;
            } else {
                // Found
                break;
            }
        }
    }

    // Return
    if (new_path.empty()) {
        return nullptr;
    } else {
        char *ret = strdup(new_path.c_str());
        ALLOC_CHECK(ret);
        return ret;
    }
}

// Hook fopen
HOOK(fopen, FILE *, (const char *filename, const char *mode)) {
    char *new_path = override_get_path(filename);
    // Open File
    ensure_fopen();
    FILE *file = real_fopen(new_path != nullptr ? new_path : filename, mode);
    // Free Data
    if (new_path != nullptr) {
        free(new_path);
    }
    // Return File
    return file;
}

// Hook fopen64
HOOK(fopen64, FILE *, (const char *filename, const char *mode)) {
    char *new_path = override_get_path(filename);
    // Open File
    ensure_fopen64();
    FILE *file = real_fopen64(new_path != nullptr ? new_path : filename, mode);
    // Free Data
    if (new_path != nullptr) {
        free(new_path);
    }
    // Return File
    return file;
}

// Init
void init_override() {
    ensure_directory(get_override_directory().c_str());
}