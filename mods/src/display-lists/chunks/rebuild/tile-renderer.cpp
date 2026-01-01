#include <malloc.h>
#include <libreborn/log.h>

#include <symbols/Tile.h>
#include <symbols/TileRenderer.h>
#include <symbols/LeafTile.h>

#include "thread.h"

// Copy an object without calling any constructors or destructors.
// THIS IS UNSAFE, use it with care.
template <typename T>
struct UnsafeCopy {
    static constexpr size_t max_size = 8192;
    uchar raw[max_size];
    T *const ptr;
    UnsafeCopy():
        raw(),
        ptr((T *) raw) {}
    void init(T *ptr_) {
        const size_t size = malloc_usable_size(ptr_);
        if (size <= 0 || size > max_size) {
            IMPOSSIBLE();
        }
        memcpy(raw, (const void *) ptr_, size);
    }
};

// Un-Safely Copy A Tile
static Tile *copy_tile(Tile *tile) {
    thread_local UnsafeCopy<Tile> tile_copy_obj;
    tile_copy_obj.init(tile);
    Tile *tile_copy = tile_copy_obj.ptr;
    if (tile_copy->vtable == CustomTile::get_vtable()) {
        // Also Copy Custom Data
        CustomTile *data = custom_get<CustomTile>(tile_copy);
        thread_local UnsafeCopy<CustomTile> data_copy_obj;
        data_copy_obj.init(data);
        __custom_link(tile_copy, data_copy_obj.ptr);
    }
    return tile_copy;
}

// Render a single tile without interfering with the main thread.
bool _render_tile(TileRenderer *tile_renderer, Tile *tile, const int x, const int y, const int z) {
    // Get Tile
    Tile *tile_copy = nullptr;
    if (tile->vtable == (const Tile::VTable *) LeafTile::VTable::base) {
        // Special Case: LeafTile's (terribly coded) getTexture() method breaks when copied.
        // Thankfully, it does not need to be copied as it has a fixed shape.
        tile_copy = tile;
    } else {
        // Copy The Tile
        tile_copy = copy_tile(tile);
    }
    // Render Tile
    return tile_renderer->tesselateInWorld(tile_copy, x, y, z);
}