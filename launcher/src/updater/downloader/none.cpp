#include <libreborn/util/exec.h>
#include <libreborn/log.h>
#include <libreborn/config.h>

#include "../updater.h"

// "Downloader"
std::optional<Updater::Downloader> Updater::get_downloader(const bool is_ui) const {
    std::string url = reborn_config.updater.web_url;
    add_version_to_url(url, latest_version);
    INFO("Update URL: %s", url.c_str());
    if (is_ui) {
        open_url(url);
    }
    return std::nullopt;
}
void Updater::restart() {
    // Not Supported
    IMPOSSIBLE();
}