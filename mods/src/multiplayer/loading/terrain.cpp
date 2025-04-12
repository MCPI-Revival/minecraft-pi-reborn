#include <symbols/minecraft.h>
#include <libreborn/patch.h>

#include "internal.h"

// Prevent Terrain Generation When Accessing Chunks
bool _inhibit_terrain_generation = false;
template <typename... Args>
static void block_terrain_generation_injection(const std::function<void(Args...)> &original, Args... args) {
    if (_inhibit_terrain_generation) {
        return;
    }
    // Call Original Method
    original(std::forward<Args>(args)...);
}
static LevelChunk *RandomLevelSource_getChunk_injection(RandomLevelSource_getChunk_t original, RandomLevelSource *self, const int chunk_x, const int chunk_z) {
    // Call Original Method
    LevelChunk *ret = original(self, chunk_x, chunk_z);
    // Mark Terrain As Generated
    if (_inhibit_terrain_generation) {
        ret->done_generating = true;
    }
    // Return
    return ret;
}

// Init
void _init_multiplayer_loading_terrain() {
    // Allow Blocking Terrain Generation
    overwrite_calls(RandomLevelSource_buildSurface, block_terrain_generation_injection<RandomLevelSource *, int, int, uchar *, Biome **>);
    overwrite_calls(RandomLevelSource_prepareHeights, block_terrain_generation_injection<RandomLevelSource *, int, int, uchar *, void *, float *>);
    overwrite_calls(LevelChunk_recalcHeightmap, block_terrain_generation_injection<LevelChunk *>);
    overwrite_calls(RandomLevelSource_getChunk, RandomLevelSource_getChunk_injection);
}