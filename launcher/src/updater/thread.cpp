#include <pthread.h>

#include <libreborn/log.h>

#include "updater.h"

// Wait For Thread
void Updater::wait() {
    if (is_running) {
        pthread_join(thread, nullptr);
        if (!is_done) {
            IMPOSSIBLE();
        }
        is_running = false;
    }
}
Updater::~Updater() {
    wait();
}
void Updater::tick() {
    if (is_running && is_done) {
        wait();
        if (is_running) {
            IMPOSSIBLE();
        }
        log_error(false);
    }
}

// Start Thread
struct thread_data {
    Updater *self = nullptr;
    std::function<void(Updater *)> func;
};
void *Updater::thread_func(void *user_data) {
    // Get Data
    const thread_data *data = (const thread_data *) user_data;
    // Run
    data->func(data->self);
    // Clean Up
    data->self->is_done = true;
    delete data;
    return nullptr;
}
void Updater::start_thread(const std::function<void(Updater *)> &func) {
    // Check
    if (is_running) {
        IMPOSSIBLE();
    }
    is_running = true;
    is_done = false;
    // Start
    thread_data *data = new thread_data;
    data->self = this;
    data->func = func;
    pthread_create(&thread, nullptr, thread_func, data);
}