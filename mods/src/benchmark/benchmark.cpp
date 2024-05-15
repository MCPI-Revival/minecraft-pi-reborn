#include <ctime>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <media-layer/core.h>
#include <SDL/SDL.h>

#include <mods/init/init.h>
#include <mods/compat/compat.h>
#include <mods/misc/misc.h>

// Constants
#define NANOSECONDS_IN_SECOND 1000000000ll

// Config
#define BENCHMARK_GAME_MODE 0 // Survival Mode
#define BENCHMARK_SEED 2048 // Random Number
#define BENCHMARK_WORLD_NAME "_Benchmark" // Random Number
#define BENCHMARK_LENGTH (180ll * NANOSECONDS_IN_SECOND) // 3 Minutes
#define BENCHMARK_ROTATION_INTERVAL ((long long int) (0.02f * NANOSECONDS_IN_SECOND))
#define BENCHMARK_ROTATION_AMOUNT 10

// Create/Start World
static void start_world(Minecraft *minecraft) {
    // Log
    INFO("Loading Benchmark");

    // Specify Level Settings
    LevelSettings settings;
    settings.game_type = BENCHMARK_GAME_MODE;
    settings.seed = BENCHMARK_SEED;

    // Delete World If It Already Exists
    LevelStorageSource *level_source = minecraft->getLevelSource();
    std::string name = BENCHMARK_WORLD_NAME;
    level_source->deleteLevel(&name);

    // Select Level
    minecraft->selectLevel(&name, &name, &settings);

    // Open ProgressScreen
    ProgressScreen *screen = alloc_ProgressScreen();
    ALLOC_CHECK(screen);
    screen = screen->constructor();
    minecraft->setScreen((Screen *) screen);
}

// Track Frames
#ifndef MCPI_HEADLESS_MODE
static unsigned long long int frames = 0;
HOOK(media_swap_buffers, void, ()) {
    ensure_media_swap_buffers();
    real_media_swap_buffers();
    frames++;
}
#endif

// Track Ticks
static unsigned long long int ticks = 0;
static void Minecraft_tick_injection(__attribute__((unused)) Minecraft *minecraft) {
    ticks++;
}

// Get Time
static long long int get_time() {
    timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long long int a = (long long int) ts.tv_nsec;
    long long int b = ((long long int) ts.tv_sec) * NANOSECONDS_IN_SECOND;
    return a + b;
}

// Store Time When World Loaded
static int world_loaded = 0;
static long long int world_loaded_time;
#ifndef MCPI_HEADLESS_MODE
static unsigned long long int world_loaded_frames;
#endif
static unsigned long long int world_loaded_ticks;

// Last Logged Status Update
static int32_t last_logged_status = -1;

// Runs Every Tick
static bool loaded = false;
static bool exit_requested = false;
static void Minecraft_update_injection(Minecraft *minecraft) {
    // Create/Start World
    if (!loaded) {
        start_world(minecraft);
        loaded = true;
    }

    // Detect World Loaded
    if (!world_loaded && minecraft->isLevelGenerated()) {
        world_loaded = 1;
        world_loaded_time = get_time();
#ifndef MCPI_HEADLESS_MODE
        world_loaded_frames = frames;
#endif
        world_loaded_ticks = ticks;
    }

    // Run Benchmark
    if (!exit_requested && world_loaded) {
        // Get Time
        long long int current_time = get_time() - world_loaded_time;
#ifndef MCPI_HEADLESS_MODE
        unsigned long long int current_frames = frames - world_loaded_frames;
#endif
        unsigned long long int current_ticks = ticks - world_loaded_ticks;

        // Log
        int32_t status = (((double) current_time) / ((double) BENCHMARK_LENGTH)) * 100;
        if (status > 100) {
            status = 100;
        }
        if (is_progress_difference_significant(status, last_logged_status)) {
            INFO("Benchmark Status: %i%%", status);
            last_logged_status = status;
        }

        // Rotate Player
        static long long int rotate_point = BENCHMARK_ROTATION_INTERVAL;
        if (current_time >= rotate_point) {
            SDL_Event event;
            event.type = SDL_MOUSEMOTION;
            event.motion.x = 0;
            event.motion.y = 0;
            event.motion.xrel = BENCHMARK_ROTATION_AMOUNT;
            event.motion.yrel = 0;
            SDL_PushEvent(&event);
            // Reset Rotation Timer
            rotate_point += BENCHMARK_ROTATION_INTERVAL;
        }

        // Check If Benchmark Is Over
        if (current_time >= BENCHMARK_LENGTH) {
            // Request Exit
            compat_request_exit();
            // Disable Special Behavior After Requesting Exit
            exit_requested = true;

            // Calculate FPS & TPS
            INFO("Benchmark Completed");
            INFO("    Total Time: %lld Nanoseconds", current_time);
#ifndef MCPI_HEADLESS_MODE
            static double frames_per_nanosecond = ((double) current_frames) / ((double) current_time);
            static double frames_per_second = frames_per_nanosecond * NANOSECONDS_IN_SECOND;
            INFO("    FPS: %f (%llu Total Frames)", frames_per_second, current_frames);
#endif
            static double ticks_per_nanosecond = ((double) current_ticks) / ((double) current_time);
            static double ticks_per_second = ticks_per_nanosecond * NANOSECONDS_IN_SECOND;
            INFO("    TPS: %f (%llu Total Ticks)", ticks_per_second, current_ticks);
        }
    }
}

// Init Benchmark
void init_benchmark() {
    // --benchmark: Activate Benchmark
    bool active = getenv("_MCPI_BENCHMARK") != nullptr;
    if (active) {
        misc_run_on_update(Minecraft_update_injection);
        // Track Ticks
        misc_run_on_tick(Minecraft_tick_injection);
        // Disable Interaction
        media_set_interactable(0);
        // Disable V-Sync
        media_disable_vsync();
    }
}
