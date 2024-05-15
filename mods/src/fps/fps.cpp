#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <mods/init/init.h>
#include <mods/misc/misc.h>
#include <mods/feature/feature.h>
#include <mods/fps/fps.h>

// Track FPS
#define NANOSECONDS_IN_SECOND 1000000000ll
static long long int get_time() {
    timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long long int a = (long long int) ts.tv_nsec;
    long long int b = ((long long int) ts.tv_sec) * NANOSECONDS_IN_SECOND;
    return a + b;
}
double fps = 0;
static void update_fps(__attribute__((unused)) Minecraft *minecraft) {
    static bool is_last_update_time_set = false;
    static long long int last_update_time;
    long long int time = get_time();
    if (is_last_update_time_set) {
        long long int update_time = time - last_update_time;
        fps = ((double) NANOSECONDS_IN_SECOND) / ((double) update_time);
    } else {
        is_last_update_time_set = true;
    }
    last_update_time = time;
}

// Print FPS
static void print_fps(__attribute__((unused)) Minecraft *minecraft) {
    // Track Ticks
    static int ticks = 0;
    ticks++;
    if (ticks == 20) {
        // One Second Has Passed, Reset
        ticks = 0;
    }

    // Print
    if (ticks == 0) {
        INFO("FPS: %f", fps);
    }
}

// Init
void init_fps() {
    if (feature_has("Track FPS", server_disabled)) {
        misc_run_on_update(update_fps);
        misc_run_on_tick(print_fps);
    }
}

