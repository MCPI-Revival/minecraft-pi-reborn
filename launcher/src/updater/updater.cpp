#include <pthread.h>
#include <libreborn/log.h>

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

// Run
static void *update_thread(void *data) {
    Updater *updater = (Updater *) data;
    updater->update();
    return nullptr;
}
void Updater::start() {
    switch (status) {
        case UpdateStatus::NOT_STARTED: {
            status = UpdateStatus::CHECKING;
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
    status = UpdateStatus::CHECKING;
    update();
}