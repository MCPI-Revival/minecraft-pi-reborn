#include <libreborn/patch.h>

#include <symbols/minecraft.h>
#include <GLES/gl.h>

#include <mods/display-lists/display-lists.h>

#include "../internal.h"

// Create/Delete Display Lists
static void create_lists(LevelRenderer *self) {
    self->display_lists = media_glGenLists(self->num_buffers);
}
static void delete_lists(const LevelRenderer *self) {
    media_glDeleteLists(self->display_lists, self->num_buffers);
}
static LevelRenderer *LevelRenderer_constructor_injection(LevelRenderer_constructor_t original, LevelRenderer *self, Minecraft *minecraft) {
    // Call Original Method
    original(self, minecraft);
    // Create Display Lists
    create_lists(self);
    return self;
}
static void LevelRenderer_onGraphicsReset_injection(LevelRenderer_onGraphicsReset_t original, LevelRenderer *self) {
    // Re-Create Display Lists
    create_lists(self);
    // Call Original Method
    original(self);
}
static LevelRenderer *LevelRenderer_destructor_injection(LevelRenderer_destructor_complete_t original, LevelRenderer *self) {
    // Delete Display Lists
    delete_lists(self);
    // Call Original Method
    return original(self);
}

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
        if (!chunk->is_empty && !chunk->is_layer_empty[a] && chunk->visible) {
            const GLuint list = chunk->getList(a);
            if (list > 0) {
                display_lists[num_display_lists++] = list;
            }
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

// Init
void _init_display_lists_chunks_render() {
    // Create/Delete Display Lists
    overwrite_calls(LevelRenderer_constructor, LevelRenderer_constructor_injection);
    overwrite_calls(LevelRenderer_onGraphicsReset, LevelRenderer_onGraphicsReset_injection);
    overwrite_calls(LevelRenderer_destructor_complete, LevelRenderer_destructor_injection);

    // Disable Creating/Deleting Buffers
    for (const uint32_t addr : {0x4e6f8, 0x4e51c, 0x4e564}) {
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) addr, nop_patch);
    }

    // Render Chunks
    enabled = true;
    overwrite_calls(LevelRenderer_renderChunks, LevelRenderer_renderChunks_injection);
}