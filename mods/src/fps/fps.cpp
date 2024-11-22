#include <libreborn/log.h>
#include <libreborn/config.h>

#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/fps/fps.h>

// Get Time
#define NANOSECONDS_IN_SECOND 1000000000ll
static long long int get_time() {
    timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    const long long int a = ts.tv_nsec;
    const long long int b = ((long long int) ts.tv_sec) * NANOSECONDS_IN_SECOND;
    return a + b;
}

// Tracker
struct Tracker {
    // State
    long long int counter = 0;
    long long int last_time = get_time();
    // Properties
    double &out;
    const std::function<void()> callback;
    // Constructor
    Tracker(double &out_, const std::function<void()> &callback_):
        out(out_),
        callback(callback_) {}
    // Update
    void update() {
        // Update Counter
        counter++;
        // Get Delta
        const long long int time = get_time();
        const long long int delta = time - last_time;
        const double delta_seconds = double(delta) / double(NANOSECONDS_IN_SECOND);
        // Calculate FPS
        if (delta_seconds >= 1) {
            out = double(counter) / delta_seconds;
            counter = 0;
            last_time = time;
            // Callback
            if (callback) {
                callback();
            }
        }
    }
};

// Track FPS
static bool log_fps;
double fps = 0;
static void fps_callback() {
    // Log
    if (log_fps) {
        INFO("FPS: %f", fps);
    }
}
static Tracker fps_tracker(fps, fps_callback);
static void update_fps() {
    fps_tracker.update();
}

// Track TPS
double tps = 0;
static Tracker tps_tracker(tps, nullptr);
static void update_tps(__attribute__((unused)) Minecraft *minecraft) {
    tps_tracker.update();
}

// Init
void init_fps() {
    if (!reborn_is_headless()) {
        misc_run_on_swap_buffers(update_fps);
        log_fps = feature_has("Log FPS", server_disabled);
    }
    misc_run_on_tick(update_tps);
}
