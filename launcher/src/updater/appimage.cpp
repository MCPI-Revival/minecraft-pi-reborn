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
template <typename... Args>
static std::optional<std::string> run_wget(Args... args) {
    exit_status_t status = 0;
    const char *const command[] = {"wget", "-O", std::forward<Args>(args)..., nullptr};
    const std::vector<unsigned char> *output = run_command(command, &status);
    std::string output_str = (const char *) output->data();
    delete output;
    if (!is_exit_status_success(status)) {
        return std::nullopt;
    } else {
        return output_str;
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
    const std::optional<std::string> out = run_wget(new_appimage_path.c_str(), url.c_str());
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
    const char *const command[] = {get_appimage_path().c_str(), nullptr};
    safe_execvpe(command);
}