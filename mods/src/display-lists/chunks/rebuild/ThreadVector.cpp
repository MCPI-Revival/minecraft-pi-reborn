#include <ranges>

#include "ThreadVector.h"

// Add Data
void ThreadVector::add(void *key, void *value) {
    pthread_mutex_lock(&mutex);
    data[key] = value;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

// Receive Data
void ThreadVector::receive(std::vector<void *> &vec, const bool wait) {
    pthread_mutex_lock(&mutex);
    while (wait && data.empty()) {
        pthread_cond_wait(&cond, &mutex);
    }
    vec.clear();
    for (void *ptr : data | std::views::values) {
        vec.push_back(ptr);
    }
    data.clear();
    pthread_mutex_unlock(&mutex);
}
