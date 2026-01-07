#pragma once

#include <pthread.h>
#include <unordered_map>
#include <vector>

// Used For Inter-Thread Communication
class ThreadVector {
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    std::unordered_map<void *, void *> data = {};
public:
    void add(void *key, void *value);
    void receive(std::vector<void *> &vec, bool wait);
};