#include <cstdlib>
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <unistd.h>

#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>

#include "cache.h"
#include "configuration.h"

// Get Cache Path
static std::string get_cache_path() {
    return home_get() + "/.launcher-cache";
}

// Load
template <typename T>
static T simple_read(std::ifstream &stream) {
    T out;
    stream.read((char *) &out, sizeof(T));
    return out;
}
template <>
std::string simple_read<std::string>(std::ifstream &stream) {
    std::string out;
    if (!std::getline(stream, out, '\0')) {
        out = "";
    }
    return out;
}
static void read_cache(std::ifstream &stream, State &ret) {
    // Cache Version
    const unsigned char cache_version = simple_read<unsigned char>(stream);
    if (stream.eof()) {
        // Unable To Read Version
        WARN("Unable To Read Launcher Cache Version");
        return;
    }

    // Support Older Versions
    bool load_new_fields = true;
    if (cache_version == 0) {
        // Pre-v3.0.0 Cache
        load_new_fields = false;
    } else if (cache_version != (unsigned char) CACHE_VERSION) {
        // Invalid Version
        WARN("Invalid Launcher Cache Version (Expected: %i, Actual: %i)", CACHE_VERSION, (int) cache_version);
        return;
    }

    // Load Username And Render Distance
    State state;
    state.username = simple_read<std::string>(stream);
    state.render_distance = simple_read<std::string>(stream);
    if (load_new_fields) {
        state.gui_scale = simple_read<float>(stream);
        state.servers.load(simple_read<std::string>(stream));
    }

    // Load Feature Flags
    std::unordered_map<std::string, bool> flags;
    while (!stream.eof()) {
        std::string flag = simple_read<std::string>(stream);
        flags[flag] = simple_read<bool>(stream);
        stream.peek();
    }
    state.flags.from_cache(flags);

    // Check For Error
    if (!stream) {
        WARN("Failure While Loading Launcher Cache");
        return;
    }

    // Success
    ret = state;
}
State load_cache() {
    // Log
    DEBUG("Loading Launcher Cache...");

    // Return Value
    State ret;

    // Open File
    std::ifstream stream(get_cache_path(), std::ios::in | std::ios::binary);
    if (!stream) {
        // No Warning If File Doesn't Exist
        if (errno != ENOENT) {
            WARN("Unable To Open Launcher Cache For Loading");
        }
    } else {
        // Lock File
        int lock_fd = lock_file(get_cache_path().c_str());

        // Load
        read_cache(stream, ret);

        // Close
        stream.close();

        // Unlock File
        unlock_file(get_cache_path().c_str(), lock_fd);
    }

    // Return
    return ret;
}

// Save
template <typename T>
static void simple_write(std::ostream &stream, const T &val) {
    stream.write((const char *) &val, sizeof(T));
}
template <>
void simple_write<std::string>(std::ostream &stream, const std::string &val) {
    stream.write(val.c_str(), int(val.size()) + 1);
}
void write_cache(std::ostream &stream, const State &state) {
    // Save Cache Version
    constexpr unsigned char cache_version = CACHE_VERSION;
    simple_write(stream, cache_version);

    // Save Username And Render Distance
    simple_write(stream, state.username);
    simple_write(stream, state.render_distance);
    simple_write(stream, state.gui_scale);
    simple_write(stream, state.servers.to_string());

    // Save Feature Flags
    const std::unordered_map<std::string, bool> flags_cache = state.flags.to_cache();
    for (const std::pair<const std::string, bool> &it : flags_cache) {
        simple_write(stream, it.first);
        simple_write(stream, it.second);
    }
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
        const int lock_fd = lock_file(get_cache_path().c_str());

        // Write
        write_cache(stream, state);

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
