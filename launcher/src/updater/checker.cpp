#include <libreborn/util/exec.h>
#include <libreborn/config.h>
#include <libreborn/log.h>

#include "updater.h"

// Download A File
std::optional<std::string> run_wget(const std::string &dest, const std::string &url) {
    const std::vector<unsigned char> *output = download_from_internet(dest, url);
    if (output) {
        std::string output_str = (const char *) output->data();
        delete output;
        return output_str;
    } else {
        return std::nullopt;
    }
}

// Check For Update
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
static std::optional<std::string> check_for_update() {
    // Check
    const std::optional<std::string> json = run_wget("-", reborn_config.updater.json_url);
    if (!json.has_value()) {
        return std::nullopt;
    }
    std::string tag_name = extract_from_json(json.value(), "tag_name");
    if (tag_name.empty()) {
        return std::nullopt;
    }
    return tag_name;
}

// Integrate Into Updater
void Updater::check() {
    // Get Current Version
    status = UpdateStatus::CHECKING;
    const std::string current_version = reborn_config.general.version;
    INFO("Checking For Updates...");
    INFO("Current Version: %s", current_version.c_str());

    // Start Thread
    start_thread([current_version](Updater *self) {
        // Check For Updates
        const std::optional<std::string> ret = check_for_update();

        // Check Result
        if (ret.has_value()) {
            // Compare Versions
            const std::string &version = ret.value();
            INFO("Latest Version: %s", version.c_str());
            self->latest_version = version;
            const bool up_to_date = version == current_version;
            if (up_to_date) {
                INFO("Already Up-To-Date");
            } else {
                INFO("New Version Available");
            }
            self->status = up_to_date ? UpdateStatus::UP_TO_DATE : UpdateStatus::UPDATE_AVAILABLE;
        } else {
            // Error
            self->status = UpdateStatus::UNKNOWN_ERROR;
        }
    });
}