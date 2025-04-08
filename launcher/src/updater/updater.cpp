#include <pthread.h>

#include <libreborn/log.h>
#include <libreborn/config.h>

#include "updater.h"

// Instance
Updater *Updater::instance = nullptr;
Updater::Updater() {
    instance = this;
}

// Check Status
bool Updater::can_start() const {
    return status == UpdateStatus::NOT_STARTED || status == UpdateStatus::RESTART_NEEDED;
}
std::string Updater::get_status() const {
    switch (status) {
        case UpdateStatus::NOT_STARTED: return "Update";
        case UpdateStatus::RESTART_NEEDED: return "Restart!";
        case UpdateStatus::CHECKING: return "Checking...";
        case UpdateStatus::UP_TO_DATE: return "Up-To-Date";
        case UpdateStatus::DOWNLOADING: return "Downloading...";
        case UpdateStatus::ERROR: return "Error";
        default: return "";
    }
}
void Updater::log_status(const bool is_ui) const {
    if (status == UpdateStatus::ERROR) {
        CONDITIONAL_ERR(!is_ui, "Unable To Update");
    } else if (status == UpdateStatus::UP_TO_DATE) {
        INFO("Already Up-To-Date");
    } else {
        if (status != UpdateStatus::RESTART_NEEDED) {
            IMPOSSIBLE();
        }
        INFO("Update Completed");
    }
}

// Run
static void *update_thread(void *data) {
    Updater *updater = (Updater *) data;
    updater->run();
    updater->log_status(true);
    return nullptr;
}
void Updater::start() {
    switch (status) {
        case UpdateStatus::NOT_STARTED: {
            pthread_t thread;
            pthread_create(&thread, nullptr, update_thread, this);
            break;
        }
        case UpdateStatus::RESTART_NEEDED: {
            restart();
            break;
        }
        default: IMPOSSIBLE();
    }
}
void Updater::run() {
    if (status != UpdateStatus::NOT_STARTED) {
        IMPOSSIBLE();
    }
    // Check Latest Version
    status = UpdateStatus::CHECKING;
    const std::string current_version = reborn_get_version();
    INFO("Current Version: %s", current_version.c_str());
    const std::optional<std::string> latest_version = check();
    if (!latest_version.has_value()) {
        // Error
        status = UpdateStatus::ERROR;
        return;
    }
    INFO("New Version: %s", latest_version.value().c_str());
    if (latest_version.value() == current_version) {
        // Up-To-Date
        status = UpdateStatus::UP_TO_DATE;
        return;
    }
    // Download
    INFO("Downloading Update...");
    status = UpdateStatus::DOWNLOADING;
    if (!download(latest_version.value())) {
        // Error
        status = UpdateStatus::ERROR;
    }
}