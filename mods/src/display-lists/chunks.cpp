#include "internal.h"

#include <libreborn/patch.h>

#include <symbols/LevelRenderer.h>
#include <symbols/Minecraft.h>
#include <symbols/Mob.h>
#include <symbols/Chunk.h>
#include <symbols/RenderChunk.h>

#include <GLES/gl.h>

#include <mods/display-lists/display-lists.h>

// Render
#define MAX_DISPLAY_LISTS 4096
static GLuint display_lists[MAX_DISPLAY_LISTS];
static int num_display_lists = 0;
static void display_lists_renderSameAsLast(const LevelRenderer *self, const float b) {
    // Prepare Offset
    const Mob *camera = self->minecraft->camera;
    const float x = camera->old_x + ((camera->x - camera->old_x) * b);
    const float y = camera->old_y + ((camera->y - camera->old_y) * b);
    const float z = camera->old_z + ((camera->z - camera->old_z) * b);
    media_glPushMatrix();
    media_glTranslatef(-x, -y, -z);

    // Render
    media_glCallLists(num_display_lists, GL_UNSIGNED_INT, display_lists);

    // Clean Up
    media_glPopMatrix();
}
static int LevelRenderer_renderChunks_injection(MCPI_UNUSED LevelRenderer_renderChunks_t original, const LevelRenderer *self, const int start, const int end, const int a, const float b) {
    // Gather Display Lists
    num_display_lists = 0;
    for (int i = start; i < end; i++) {
        Chunk *chunk = self->chunks[i];
        // Check If Chunk Is Visible
        if (chunk->visible && !chunk->is_empty && !chunk->is_layer_empty[a]) {
            // Chunk::rebuild uses Tesselator::end when finished rebuilding.
            // This is automatically converted to an OpenGL Display List by
            // tesselator.cpp. This retrieves that Display List.
            const RenderChunk *render_chunk = chunk->getRenderChunk(a);
            const GLuint buffer = render_chunk->buffer;
            const GLuint list = _display_lists_get_for_buffer(buffer);
            display_lists[num_display_lists++] = list;
        }
    }

    // Draw
    display_lists_renderSameAsLast(self, b);

    // Return
    return num_display_lists;
}

// API
static bool enabled = false;
void LevelRenderer_renderSameAsLast(LevelRenderer *self, const float delta) {
    if (enabled) {
        display_lists_renderSameAsLast(self, delta);
    } else {
        self->render_list.render();
    }
}

// Better Empty Chunk Logic
static bool Chunk_isEmpty_injection(Chunk *self) {
    return !self->built || self->is_empty;
}

// Init
void _init_display_lists_chunks_render() {
    enabled = true;
    overwrite_calls(LevelRenderer_renderChunks, LevelRenderer_renderChunks_injection);
    replace_func(Chunk_isEmpty);
    // Disable Offsetting Rebuilt Chunks
    unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x479fc, nop_patch);
}