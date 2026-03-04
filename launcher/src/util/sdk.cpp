#include "util.h"

#include <optional>
#include <fstream>

#include <libreborn/log.h>
#include <libreborn/util/util.h>
#include <libreborn/util/io.h>
#include <libreborn/config.h>

// Path
static std::string get_sdk_root(const std::string &home) {
    return home + path_separator + "sdk";
}
static std::string get_sdk_path_home() {
    std::string sdk_dir = reborn_config.internal.sdk_dir;
    for (char &c : sdk_dir) {
        if (c == linux_path_separator) {
            c = path_separator;
        }
    }
    return get_sdk_root(home_get()) + path_separator + sdk_dir;
}
static std::string get_sdk_path_bundled(const std::string &binary_directory) {
    return get_sdk_root(binary_directory);
}

// Test Whether SDK Should Be Copied
static std::optional<std::string> get_sdk_hash(const std::string &sdk) {
    const std::string path = sdk + path_separator + ".hash";
    // Open File
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (stream) {
        std::string hash;
        // Read File
        const std::streamoff size = stream.tellg();
        stream.seekg(0, std::ifstream::beg);
        hash.resize(size);
        stream.read(hash.data(), size);
        // Close File
        stream.close();
        // Return
        return hash;
    } else {
        // Unable To Read
        return std::nullopt;
    }
}
static bool should_copy_sdk(const std::string &binary_directory) {
    // Read Hashes
    const std::optional<std::string> home_hash = get_sdk_hash(get_sdk_path_home());
    if (!home_hash.has_value()) {
        return true;
    }
    const std::optional<std::string> bundled_hash = get_sdk_hash(get_sdk_path_bundled(binary_directory));
    if (!home_hash.has_value()) {
        IMPOSSIBLE();
    }
    const bool should_copy = home_hash.value() != bundled_hash.value();
    if (!should_copy) {
        DEBUG("Skipped Unnecessary SDK Copy");
    }
    return should_copy;
}

// Log
#define LOG(is_debug, ...) \
    ({ \
        if ((is_debug)) { \
            DEBUG(__VA_ARGS__); \
        } else { \
            INFO(__VA_ARGS__); \
        } \
    })

// Copy SDK Into ~/.minecraft-pi
static void do_copy_sdk(const std::string &binary_directory, const bool force) {
    // Check If Copy Is Needed
    bool should_copy = force;
    if (!should_copy) {
        should_copy = should_copy_sdk(binary_directory);
    }
    if (!should_copy) {
        return;
    }

    // Get Paths
    const std::string src_sdk = get_sdk_path_bundled(binary_directory);
    const std::string dst_sdk = get_sdk_path_home();

    // Create Output Directory
    delete_recursively(dst_sdk, true);
    make_directory(dst_sdk);

    // Copy Directory
    copy_directory(src_sdk, dst_sdk);

    // Log
    LOG(!force, "Copied SDK To: %s", dst_sdk.c_str());
}
void copy_sdk(const std::string &binary_directory, const bool force) {
    // Lock File
    const std::string root = get_sdk_root(home_get());
    ensure_directory(root.c_str());
    const std::string lock_file_path = root + path_separator;
    const HANDLE lock_file_fd = lock_file(lock_file_path.c_str());

    // Do
    do_copy_sdk(binary_directory, force);

    // Unlock File
    unlock_file(lock_file_fd);
}