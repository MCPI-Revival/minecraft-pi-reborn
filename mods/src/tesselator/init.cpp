#include <pthread.h>

#include <mods/display-lists/display-lists.h>

#include "internal.h"

// Handle Terrain Rendering
static void Chunk_rebuild_injection(Chunk_rebuild_t original, Chunk *self) {
    CustomTesselator &t = advanced_tesselator_get();
    t.are_vertices_flat = true;
    t.enable_real_quads = display_lists_enabled_for_chunk_rendering;
    original(self);
    t.are_vertices_flat = false;
    t.enable_real_quads = false;
}

// Init
pthread_t main_thread;
void advanced_tesselator_enable() {
    static bool initialized = false;
    if (initialized) {
        return;
    }
    _tesselator_init_state();
    _tesselator_init_vertex();
    _tesselator_init_draw();
    overwrite_calls(Chunk_rebuild, Chunk_rebuild_injection);
    main_thread = pthread_self();
    initialized = true;
}

// Get
CustomTesselator &advanced_tesselator_get() {
    const bool is_main_thread = pthread_equal(pthread_self(), main_thread);
    thread_local CustomTesselator obj(is_main_thread);
    return obj;
}