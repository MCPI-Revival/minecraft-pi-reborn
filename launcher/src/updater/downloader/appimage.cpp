#include <cstdlib>
#include <optional>
#include <utility>
#include <unistd.h>
#include <sys/stat.h>

#include <libreborn/util/exec.h>
#include <libreborn/util/string.h>
#include <libreborn/config.h>

#include "../updater.h"
#include "../../util/util.h"

// Update
static bool downloader(const std::string &version) {
    // Get URL
    std::string url = reborn_config.updater.appimage_download_url;
    add_version_to_url(url, version);

    // Get Path
    const std::string appimage_path = get_appimage_path();
    const std::string new_appimage_path_base = appimage_path + ".new";
    std::string new_appimage_path = new_appimage_path_base;
    int num = 1;
    while (access(new_appimage_path.c_str(), F_OK) != -1) {
        new_appimage_path = new_appimage_path_base + '.' + safe_to_string(num++);
    }

    // Download
    const std::optional<std::string> out = run_wget(new_appimage_path, url);
    bool ret = out.has_value();
    if (ret) {
        ret = chmod(new_appimage_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
    }
    if (ret) {
        ret = rename(new_appimage_path.c_str(), appimage_path.c_str()) == 0;
    }
    if (!ret) {
        unlink(new_appimage_path.c_str());
        // Error
        return false;
    }

    // Done
    return true;
}

// Integrate Into Updater
std::optional<Updater::Downloader> Updater::get_downloader(MCPI_UNUSED const bool is_ui) const {
    return downloader;
}
void Updater::restart() {
    const std::string path = get_appimage_path();
    const char *const command[] = {path.c_str(), nullptr};
    safe_execvpe(command);
}