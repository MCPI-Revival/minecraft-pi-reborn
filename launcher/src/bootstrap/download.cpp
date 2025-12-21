#include <unistd.h>
#include <sstream>
#include <fstream>

#include <libreborn/util/io.h>
#include <libreborn/util/util.h>
#include <libreborn/util/exec.h>
#include <libreborn/config.h>
#include <libreborn/log.h>
#include <sys/stat.h>

#include "bootstrap.h"
#include "../util/util.h"

// Get Game Binary Path
static std::string get_game_dir() {
    return home_get() + path_separator + "versions" + path_separator + reborn_config.game.version;
}
std::string get_game_binary_path() {
    return get_game_dir() + path_separator + "minecraft-pi";
}

// Download An Archive
static bool download_archive(const std::string &url, const std::string &dest) {
    // Download
    unlink(dest.c_str());
    const std::vector<unsigned char> *output = download_from_internet(dest, url, reborn_config.game.download_user_agent);
    if (!output) {
        return false;
    }
    delete output;

    // Convert .tar.gz.zip Into .zip
    if (dest.ends_with(".zip")) {
        // Extract
        const char *const command[] = {
            "unzip",
            "-p",
            dest.c_str(),
            nullptr
        };
        exit_status_t status;
        output = run_command(command, &status);
        if (!output || !is_exit_status_success(status)) {
            return false;
        }
        // Write To File
        std::ofstream file(dest, std::ios::binary);
        if (!file) {
            return false;
        }
        file.write((const char *) output->data(), std::streamsize(output->size()));
        if (!file) {
            return false;
        }
        file.close();
    }

    // Success
    return true;
}

// Download The Game Archive
static std::string get_filename(const std::string &url) {
    const std::string::size_type i = url.find_last_of('/');
    if (i == std::string::npos) {
        IMPOSSIBLE();
    }
    return url.substr(i + 1);
}
static std::string download_game_archive() {
    // Get Possible URLs
    const std::string urls = reborn_config.game.download_url;
    std::stringstream url_stream(urls);

    // Get First Part Of Destination File Path
    std::string dest_prefix = get_temp_dir();
    dest_prefix += std::to_string(getpid()) + '-';

    // Try All Possible URLs Until One Succeeds
    std::string dest;
    while (true) {
        std::string url;
        if (!std::getline(url_stream, url, ';')) {
            // All URLs Have Failed
            break;
        }
        const std::string dest_possible = dest_prefix + get_filename(url);
        if (download_archive(url, dest_possible)) {
            // Successfully Downloaded Archive
            dest = dest_possible;
            break;
        } else {
            // Failed To Download
            // Cleanup The Mess
            unlink(dest_possible.c_str());
        }
    }

    // Return
    if (dest.empty()) {
        ERR("Unable To Download Game");
    }
    return dest;
}

// Extract Game Archive
static void extract_game_archive(const std::string &game_dir, const std::string &archive) {
    const char *const command[] = {
        "tar",
        "-xf", archive.c_str(),
        "-C", game_dir.c_str(),
        "--strip-components", "1",
        nullptr
    };
    exit_status_t status;
    const std::vector<unsigned char> *output = run_command(command, &status);
    delete output;
    unlink(archive.c_str());
    if (!is_exit_status_success(status)) {
        ERR("Unable To Extract Game");
    }
}

// Download Game
void download_game() {
    // Download
    DEBUG("Downloading Game...");
    const std::string dest = download_game_archive();

    // Extract Game
    DEBUG("Extracting Game...");
    const std::string game_dir = get_game_dir();
    delete_recursively(game_dir, true);
    make_directory(game_dir);
    extract_game_archive(game_dir, dest);
}