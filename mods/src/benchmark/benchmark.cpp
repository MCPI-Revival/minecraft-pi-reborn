#include <ctime>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#include <media-layer/core.h>
#include <SDL/SDL.h>

#include "../init/init.h"
#include "../compat/compat.h"
#include "../misc/misc.h"

// --benchmark: Activate Benchmark
static bool active = false;
__attribute__((constructor)) static void _init_active(int argc, char *argv[]) {
    // Iterate Arguments
    for (int i = 1; i < argc; i++) {
        // Check Argument
        if (strcmp(argv[i], "--benchmark") == 0) {
            // Enabled
            active = true;
            break;
        }
    }
}

// Constants
#define NANOSECONDS_IN_SECOND 1000000000ll

// Config
#define BENCHMARK_GAME_MODE 1 // Creative Mode
#define BENCHMARK_SEED 2048 // Random Number
#define BENCHMARK_WORLD_NAME "_Benchmark" // Random Number
#define BENCHMARK_LENGTH (180ll * NANOSECONDS_IN_SECOND) // 3 Minutes
#define BENCHMARK_ROTATION_INTERVAL ((long long int) (0.02f * NANOSECONDS_IN_SECOND))
#define BENCHMARK_ROTATION_AMOUNT 10

// Create/Start World
static void start_world(unsigned char *minecraft) {
    // Log
    INFO("Loading Benchmark");

    // Specify Level Settings
    LevelSettings settings;
    settings.game_type = BENCHMARK_GAME_MODE;
    settings.seed = BENCHMARK_SEED;

    // Delete World If It Already Exists
    unsigned char *level_source = (*Minecraft_getLevelSource)(minecraft);
    unsigned char *level_source_vtable = *(unsigned char **) level_source;
    ExternalFileLevelStorageSource_deleteLevel_t ExternalFileLevelStorageSource_deleteLevel = *(ExternalFileLevelStorageSource_deleteLevel_t *) (level_source_vtable + ExternalFileLevelStorageSource_deleteLevel_vtable_offset);
    (*ExternalFileLevelStorageSource_deleteLevel)(level_source, BENCHMARK_WORLD_NAME);

    // Select Level
    (*Minecraft_selectLevel)(minecraft, BENCHMARK_WORLD_NAME, BENCHMARK_WORLD_NAME, settings);

    // Open ProgressScreen
    void *screen = ::operator new(PROGRESS_SCREEN_SIZE);
    ALLOC_CHECK(screen);
    screen = (*ProgressScreen)((unsigned char *) screen);
    (*Minecraft_setScreen)(minecraft, (unsigned char *) screen);
}

// Track Frames
static unsigned long long int frames = 0;
HOOK(media_swap_buffers, void, ()) {
    ensure_media_swap_buffers();
    (*real_media_swap_buffers)();
    frames++;
}

// Get Time
static long long int get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long long int a = (long long int) ts.tv_nsec;
    long long int b = ((long long int) ts.tv_sec) * NANOSECONDS_IN_SECOND;
    return a + b;
}

// Store Time When World Loaded
static int world_loaded = 0;
static long long int world_loaded_time;
static unsigned long long int world_loaded_frames;

// Runs Every Tick
static bool loaded = false;
static bool exit_requested = false;
static void Minecraft_update_injection(unsigned char *minecraft) {
    // Create/Start World
    if (!loaded) {
        start_world(minecraft);
        loaded = true;
    }

    // Detect World Loaded
    if (!world_loaded && (*Minecraft_isLevelGenerated)(minecraft)) {
        world_loaded = 1;
        world_loaded_time = get_time();
        world_loaded_frames = frames;
    }

    // Run Benchmark
    if (!exit_requested && world_loaded) {
        // Get Time
        long long int current_time = get_time() - world_loaded_time;
        unsigned long long int current_frames = frames - world_loaded_frames;

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

            // Calculate FPS
            static double frames_per_nanosecond = ((double) current_frames) / ((double) current_time);
            static double frames_per_second = frames_per_nanosecond * NANOSECONDS_IN_SECOND;
            INFO("Benchmark Completed After %llu Frames In %lld Nanoseconds, Average FPS: %f", current_frames, current_time, frames_per_second);
        }
    }
}

// Init Benchmark
void init_benchmark() {
    if (active) {
        misc_run_on_update(Minecraft_update_injection);
        // Disable Interaction
        media_set_interactable(0);
        // Disable V-Sync
        media_disable_vsync();
    }
}
