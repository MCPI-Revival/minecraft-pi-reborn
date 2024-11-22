#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>

#include <libreborn/libreborn.h>

#include <mods/override/override.h>
#include <mods/init/init.h>

// Hook Functions
#define HOOK_OPEN(name, return_type, mode_type) \
    HOOK(name, return_type, (const char *filename, mode_type mode)) { \
        const std::string new_path = override_get_path(filename); \
        /* Open File */ \
        return_type ret = real_##name()(!new_path.empty() ? new_path.c_str() : filename, mode); \
        /* Return */ \
        return ret; \
    }
HOOK_OPEN(fopen, FILE *, const char *)
HOOK_OPEN(fopen64, FILE *, const char *)
HOOK_OPEN(access, int, int)

// Get Override Folder
static std::string get_override_directory() {
    const std::string home_path = home_get();
    return home_path + "/overrides";
}

// Get Override Path For File (If It Exists)
std::string override_get_path(std::string filename) {
    // Custom Skin
    if (filename.starts_with("data/images/$")) {
        // Fallback Texture
        filename = "data/images/mob/char.png";
    }

    // Get Asset Override Path
    const std::string overrides = get_override_directory();

    // Data Prefix
    const std::string data_prefix = "data/";
    const int data_prefix_length = data_prefix.length();

    // Folders To Check
    const std::string asset_folders[] = {
        overrides,
        getenv(_MCPI_REBORN_ASSETS_PATH_ENV),
        getenv(_MCPI_VANILLA_ASSETS_PATH_ENV),
        ""
    };

    // Check For Override
    std::string new_path;
    if (filename.starts_with(data_prefix)) {
        // Test Asset Folders
        for (int i = 0; !asset_folders[i].empty(); i++) {
            new_path = asset_folders[i] + '/' + &filename[data_prefix_length];
            if (real_access()(new_path.c_str(), F_OK) == -1) {
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
    return new_path;
}

// Init
void init_override() {
    ensure_directory(get_override_directory().c_str());
}