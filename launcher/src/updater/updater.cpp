#include <libreborn/log.h>
#include <libreborn/config.h>

#include "updater.h"

// Constructor
Updater::Updater():
    thread(),
    is_running(false),
    is_done(false),
    status(UpdateStatus::NOT_STARTED),
    has_started_download(false) {}

// Error Logging
void Updater::log_error(const bool is_sync) const {
    if (status == UpdateStatus::UNKNOWN_ERROR) {
        CONDITIONAL_ERR(is_sync, "Unable To Update");
    }
}

// Asynchronous Mode
bool Updater::can_launch_safely() const {
    return !has_started_download;
}
bool Updater::can_start() const {
    if (is_running) {
        return false;
    }
    return status == UpdateStatus::NOT_STARTED ||
        status == UpdateStatus::UPDATE_AVAILABLE ||
        status == UpdateStatus::RESTART_NEEDED;
}
std::string Updater::get_status() const {
    switch (status) {
        case UpdateStatus::NOT_STARTED: return "Check For Updates";
        case UpdateStatus::CHECKING: return "Checking...";
        case UpdateStatus::UP_TO_DATE: return "Up-To-Date";
        case UpdateStatus::UPDATE_AVAILABLE: return "Download Update";
        case UpdateStatus::DOWNLOADING: return "Downloading...";
        case UpdateStatus::RESTART_NEEDED: return "Restart Required";
        case UpdateStatus::UNKNOWN_ERROR: return "Error";
        default: return "";
    }
}
void Updater::start() {
    if (!can_start()) {
        IMPOSSIBLE();
    }
    switch (status) {
        case UpdateStatus::NOT_STARTED: check(); break;
        case UpdateStatus::UPDATE_AVAILABLE: download(true); break;
        case UpdateStatus::RESTART_NEEDED: restart(); break;
        default: IMPOSSIBLE();
    }
}

// Synchronous Mode
void Updater::run() {
    if (status != UpdateStatus::NOT_STARTED || !can_start()) {
        IMPOSSIBLE();
    }
    check();
    wait();
    if (status == UpdateStatus::UPDATE_AVAILABLE) {
        download(false);
        wait();
        if (status == UpdateStatus::UPDATE_AVAILABLE) {
            WARN("Unable To Download Updates Automatically");
        }
    }
    log_error(true);
}

// Download Update
void Updater::download(const bool is_ui) {
    const std::optional<Downloader> downloader = get_downloader(is_ui);
    if (downloader.has_value()) {
        status = UpdateStatus::DOWNLOADING;
        has_started_download = true;
        INFO("Downloading Update...");
        start_thread([downloader](Updater *self) {
            const bool ret = downloader.value()(self->latest_version);
            if (ret) {
                INFO("Update Completed");
            }
            self->status = ret ? UpdateStatus::RESTART_NEEDED : UpdateStatus::UNKNOWN_ERROR;
        });
    }
}

// Add Version To URL
void add_version_to_url(std::string &url, const std::string &version) {
    while (true) {
        const std::string placeholder = reborn_config.updater.version_placeholder;
        const std::string::size_type pos = url.find(placeholder);
        if (pos == std::string::npos) {
            break;
        }
        url.replace(pos, placeholder.size(), version);
    }
}