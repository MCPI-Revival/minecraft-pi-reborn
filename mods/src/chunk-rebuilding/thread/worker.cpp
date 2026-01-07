#include "worker.h"
#include "messages.h"

#include <unistd.h>

#include <libreborn/log.h>

#include <symbols/Level.h>

// Start/Stop
RebuildWorker::RebuildWorker(const int seed_):
    thread(),
    seed(seed_)
{
    // Start Thread
    const int ret = pthread_create(&thread, nullptr, run, this);
    if (ret != 0) {
        IMPOSSIBLE();
    }
}
RebuildWorker::~RebuildWorker() {
    // Signal Stop
    input.add(nullptr, nullptr);
    // Wait For Thread To Stop
    pthread_join(thread, nullptr);
    // Free Remaining Messages
    std::vector<void *> data;
    output.receive(data, false);
    for (const void *msg : data) {
        const rebuilt_chunk_data *chunk = (const rebuilt_chunk_data *) msg;
        _free_rebuilt_chunk_data(chunk);
    }
}

// Track Running Workers
static std::vector<RebuildWorker *> workers;

// Send A Message
static int next_worker;
void _start_chunk_rebuild(void *key, void *value) {
    workers.at(next_worker)->input.add(key, value);
    next_worker++;
    next_worker %= workers.size();
}

// Receive Messages
void _receive_rebuilt_chunks(std::vector<void *> &data) {
    data.clear();
    for (RebuildWorker *worker : workers) {
        std::vector<void *> tmp;
        worker->output.receive(tmp, false);
        data.insert(data.end(), tmp.begin(), tmp.end());
    }
}

// Recommended Worker Count
static int get_worker_count() {
    // Get Core Count
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    // Do Not Use All Cores
    nproc /= 4;
    // At Least One Worker
    constexpr long min = 1;
    if (nproc < min) {
        return min;
    }
    // Return
    return int(nproc);
}

// Start All Workers
void _start_chunk_rebuild_thread(Level *level) {
    if (!workers.empty()) {
        // Workers Have Already Started
        IMPOSSIBLE();
    }
    const int worker_count = get_worker_count();
    DEBUG("Starting %i Chunk Rebuild Workers...", worker_count);
    const int seed = level->getSeed();
    for (int i = 0; i < worker_count; i++) {
        workers.emplace_back(new RebuildWorker(seed));
    }
    next_worker = 0;
}

// Stop All Workers
void _stop_chunk_rebuild_thread() {
    if (workers.empty()) {
        return;
    }
    DEBUG("Stopping Chunk Rebuild Workers...");
    for (const RebuildWorker *worker : workers) {
        delete worker;
    }
    workers.clear();
}