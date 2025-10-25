#include <unistd.h>

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

// Download Game
static std::string get_filename(const std::string &url) {
    const std::string::size_type i = url.find_last_of('/');
    if (i == std::string::npos) {
        IMPOSSIBLE();
    }
    return url.substr(i + 1);
}
void download_game() {
    // Download
    DEBUG("Downloading Game...");
    const std::string url = reborn_config.game.download_url;
    std::string dest = get_temp_dir();
    dest += std::to_string(getpid()) + '-';
    dest += get_filename(url);
    const std::vector<unsigned char> *output = download_from_internet(dest, url);
    if (!output) {
        ERR("Unable To Download Game");
    }
    delete output;

    // Extract Game
    DEBUG("Extracting Game...");
    const std::string game_dir = get_game_dir();
    delete_recursively(game_dir, true);
    make_directory(game_dir);
    const char *const command[] = {
        "tar",
        "-xf", dest.c_str(),
        "-C", game_dir.c_str(),
        "--strip-components", "1",
        nullptr
    };
    exit_status_t status;
    output = run_command(command, &status);
    delete output;
    unlink(dest.c_str());
    if (!is_exit_status_success(status)) {
        ERR("Unable To Extract Game");
    }
}