#include <cstdlib>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include <libreborn/log.h>
#include <libreborn/util.h>

#include "cache.h"
#include "configuration.h"

// Get Cache Path
static std::string get_cache_path() {
    return home_get() + "/.launcher-cache";
}

// Load
launcher_cache empty_cache = {
    .username = DEFAULT_USERNAME,
    .render_distance = DEFAULT_RENDER_DISTANCE,
    .feature_flags = {}
};
launcher_cache load_cache() {
    // Log
    DEBUG("Loading Launcher Cache...");

    // Return Value
    launcher_cache ret = empty_cache;

    // Open File
    std::ifstream stream(get_cache_path(), std::ios::in | std::ios::binary);
    if (!stream) {
        // Fail
        struct stat s = {};
        // No Warning If File Doesn't Exist
        if (stat(get_cache_path().c_str(), &s) == 0) {
            WARN("Unable To Open Launcher Cache For Loading");
        }
    } else {
        // Lock File
        int lock_fd = lock_file(get_cache_path().c_str());

        // Check Version
        unsigned char cache_version;
        stream.read((char *) &cache_version, 1);
        if (stream.eof()) {
            // Unable To Read Version
            WARN("Unable To Read Launcher Cache Version");
        } else if (cache_version != (unsigned char) CACHE_VERSION) {
            // Invalid Version
            WARN("Invalid Launcher Cache Version (Expected: %i, Actual: %i)", CACHE_VERSION, (int) cache_version);
        } else {
            // Load Username And Render Distance
            launcher_cache cache;
            std::getline(stream, cache.username, '\0');
            std::getline(stream, cache.render_distance, '\0');

            // Load Feature Flags
            std::string flag;
            while (!stream.eof() && std::getline(stream, flag, '\0')) {
                if (!flag.empty()) {
                    bool is_enabled = false;
                    stream.read((char *) &is_enabled, sizeof(bool));
                    cache.feature_flags[flag] = is_enabled;
                }
                stream.peek();
            }

            // Finish
            if (!stream) {
                // Fail
                WARN("Failure While Loading Launcher Cache");
            } else {
                // Success
                ret = cache;
            }
        }

        // Close
        stream.close();

        // Unlock File
        unlock_file(get_cache_path().c_str(), lock_fd);
    }

    // Return
    return ret;
}

// Save
static void write_env_to_stream(std::ofstream &stream, const std::string &value) {
    stream.write(value.c_str(), int(value.size()) + 1);
}
void save_cache(const State &state) {
    // Log
    DEBUG("Saving Launcher Cache...");

    // Open File
    std::ofstream stream(get_cache_path(), std::ios::out | std::ios::binary);
    if (!stream) {
        // Fail
        WARN("Unable To Open Launcher Cache For Saving");
    } else {
        // Lock File
        int lock_fd = lock_file(get_cache_path().c_str());

        // Save Cache Version
        constexpr unsigned char cache_version = CACHE_VERSION;
        stream.write((const char *) &cache_version, 1);

        // Save Username And Render Distance
        write_env_to_stream(stream, state.username);
        write_env_to_stream(stream, state.render_distance);

        // Save Feature Flags
        const std::unordered_map<std::string, bool> flags_cache = state.flags.to_cache();
        for (const std::pair<const std::string, bool> &it : flags_cache) {
            stream.write(it.first.c_str(), int(it.first.size()) + 1);
            stream.write((const char *) &it.second, sizeof(bool));
        }

        // Finish
        stream.close();
        if (!stream) {
            WARN("Failure While Saving Launcher Cache");
        }

        // Unlock File
        unlock_file(get_cache_path().c_str(), lock_fd);
    }
}

// Wipe Cache
void wipe_cache() {
    // Log
    INFO("Wiping Launcher Cache...");

    // Unlink File
    const int ret = unlink(get_cache_path().c_str());
    if (ret != 0 && errno != ENOENT) {
        WARN("Failure While Wiping Cache: %s", strerror(errno));
    }
}
