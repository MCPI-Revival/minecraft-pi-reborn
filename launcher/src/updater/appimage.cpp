#include <cstdlib>
#include <optional>
#include <utility>
#include <unistd.h>
#include <sys/stat.h>

#include <libreborn/log.h>
#include <libreborn/util/exec.h>
#include <libreborn/config.h>

#include "updater.h"

// Implement
struct AppImageUpdater final : Updater {
    void update() override;
    void restart() override;
    static AppImageUpdater instance;
};
AppImageUpdater AppImageUpdater::instance;

// Update
template <typename... Args>
static std::optional<std::string> run_wget(Args... args) {
    int status = 0;
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
static const char *get_appimage_path() {
    const char *path = getenv("APPIMAGE");
    if (path == nullptr) {
        IMPOSSIBLE();
    }
    return path;
}
void AppImageUpdater::update() {
    // Check
    if (status != UpdateStatus::CHECKING) {
        IMPOSSIBLE();
    }
    const std::optional<std::string> json = run_wget("-", MCPI_APPIMAGE_JSON_URL);
    if (!json.has_value()) {
        status = UpdateStatus::ERROR;
        return;
    }
    const std::string tag_name = extract_from_json(json.value(), "tag_name");

    // Check Version
    if (tag_name == MCPI_VERSION) {
        status = UpdateStatus::UP_TO_DATE;
        return;
    }

    // Get URL
    std::string url = MCPI_APPIMAGE_DOWNLOAD_URL;
    while (true) {
        const std::string placeholder = MCPI_APPIMAGE_VERSION_PLACEHOLDER;
        const std::string::size_type pos = url.find(placeholder);
        if (pos == std::string::npos) {
            break;
        }
        url.replace(pos, placeholder.size(), tag_name);
    }

    // Get Path
    const char *appimage_path = get_appimage_path();
    const std::string new_appimage_path_base = std::string(appimage_path) + ".new";
    std::string new_appimage_path = new_appimage_path_base;
    int num = 1;
    while (access(new_appimage_path.c_str(), F_OK) != -1) {
        new_appimage_path = new_appimage_path_base + '.' + std::to_string(num++);
    }

    // Download
    status = UpdateStatus::DOWNLOADING;
    const std::optional<std::string> out = run_wget(new_appimage_path.c_str(), url.c_str());
    bool ret = out.has_value();
    if (ret) {
        ret = chmod(new_appimage_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
    }
    if (ret) {
        ret = rename(new_appimage_path.c_str(), appimage_path) == 0;
    }
    if (!ret) {
        unlink(new_appimage_path.c_str());
        status = UpdateStatus::ERROR;
        return;
    }

    // Done
    status = UpdateStatus::RESTART_NEEDED;
}

// Restart
void AppImageUpdater::restart() {
    const char *const command[] = {get_appimage_path(), nullptr};
    safe_execvpe(command, environ);
}