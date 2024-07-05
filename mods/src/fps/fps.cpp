#include <libreborn/libreborn.h>
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

// Track FPS
static bool log_fps;
double fps = 0;
static void update_fps() {
    // Track Frames
    static long long int frames = 0;
    frames++;
    // Get Delta
    const long long int time = get_time();
    static long long int last_time = time;
    const long long int delta = time - last_time;
    const double delta_seconds = double(delta) / double(NANOSECONDS_IN_SECOND);
    // Calculate FPS
    if (delta_seconds >= 1) {
        fps = double(frames) / delta_seconds;
        frames = 0;
        last_time = time;
        // Log
        if (log_fps) {
            INFO("FPS: %f", fps);
        }
    }
}

// Init
void init_fps() {
    misc_run_on_swap_buffers(update_fps);
    log_fps = feature_has("Log FPS", server_disabled);
}
