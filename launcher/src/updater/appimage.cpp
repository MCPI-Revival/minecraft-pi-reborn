#include <cstdlib>
#include <optional>
#include <utility>
#include <unistd.h>
#include <sys/stat.h>

#include <libreborn/util/exec.h>
#include <libreborn/util/string.h>
#include <libreborn/config.h>

#include "updater.h"
#include "../util/util.h"

// Implement
struct AppImageUpdater final : Updater {
    std::optional<std::string> check() override;
    bool download(const std::string &version) override;
    void restart() override;
    static AppImageUpdater instance;
};
AppImageUpdater AppImageUpdater::instance;

// Update
static std::optional<std::string> run_wget(const std::string &dest, const std::string &url) {
    const std::vector<unsigned char> *output = download_from_internet(dest, url);
    if (output) {
        std::string output_str = (const char *) output;
        delete output;
        return output_str;
    } else {
        return std::nullopt;
    }
}
static std::string extract_from_json(const std::string &json_str, const std::string &key) {
    std::string::size_type pos = json_str.find(key);
    std::array<std::string::size_type, 3> indices = {};
    unsigned int i = 0;
    while (true) {
        if (pos == std::string::npos) {
            return "";
        }
        if (i >= indices.size()) {
            break;
        }
        pos = json_str.find('"', pos) + 1;
        indices[i++] = pos;
    }
    const std::string::size_type start = indices[1];
    const std::string::size_type end = indices[2];
    return json_str.substr(start, end - start - 1);
}
std::optional<std::string> AppImageUpdater::check() {
    // Check
    const std::optional<std::string> json = run_wget("-", reborn_config.appimage.json_url);
    if (!json.has_value()) {
        return std::nullopt;
    }
    std::string tag_name = extract_from_json(json.value(), "tag_name");
    if (tag_name.empty()) {
        return std::nullopt;
    }
    return tag_name;
}
bool AppImageUpdater::download(const std::string &version) {
    // Get URL
    std::string url = reborn_config.appimage.download_url;
    while (true) {
        const std::string placeholder = reborn_config.appimage.version_placeholder;
        const std::string::size_type pos = url.find(placeholder);
        if (pos == std::string::npos) {
            break;
        }
        url.replace(pos, placeholder.size(), version);
    }

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

// Restart
void AppImageUpdater::restart() {
    const std::string path = get_appimage_path();
    const char *const command[] = {path.c_str(), nullptr};
    safe_execvpe(command);
}