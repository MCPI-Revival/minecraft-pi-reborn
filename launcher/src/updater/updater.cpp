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
    return status == NOT_STARTED || status == RESTART_NEEDED;
}
std::string Updater::get_status() const {
    switch (status) {
        case NOT_STARTED: return "Update";
        case RESTART_NEEDED: return "Restart!";
        case CHECKING: return "Checking...";
        case UP_TO_DATE: return "Up-To-Date";
        case DOWNLOADING: return "Downloading...";
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
        case NOT_STARTED: {
            pthread_t thread;
            pthread_create(&thread, nullptr, update_thread, this);
            break;
        }
        case RESTART_NEEDED: {
            restart();
            break;
        }
        default: IMPOSSIBLE();
    }
}