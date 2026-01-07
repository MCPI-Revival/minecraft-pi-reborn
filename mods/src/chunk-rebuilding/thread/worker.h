#pragma once

#include <pthread.h>

#include "ThreadVector.h"

// A Single Worker
struct RebuildWorker {
    // The Thread
    pthread_t thread;

    // Start/Stop
    explicit RebuildWorker(int seed_);
    ~RebuildWorker();

    // Input/Output
    ThreadVector input;
    ThreadVector output;

    // Properties
    const int seed;

private:
    // The Main Function
    static void *run(void *arg);
};

// Start/Stop
MCPI_INTERNAL void _stop_chunk_rebuild_thread();
struct Level;
MCPI_INTERNAL void _start_chunk_rebuild_thread(Level *level);

// Receive Messages From All Workers
MCPI_INTERNAL void _receive_rebuilt_chunks(std::vector<void *> &data);

// Send A Message To A Worker
MCPI_INTERNAL void _start_chunk_rebuild(void *key, void *value);