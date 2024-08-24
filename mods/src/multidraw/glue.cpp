#include <GLES/gl.h>

#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>
#include <media-layer/core.h>

#include <mods/init/init.h>
#include <mods/feature/feature.h>
#include <mods/multidraw/multidraw.h>

#include "storage.h"

// Fake Buffer IDs To Correspond To The Multidraw Storage
#define MULTIDRAW_BASE 1073741823

// Setup
static Storage *storage = nullptr;
static void setup_multidraw(int chunks, GLuint *buffers) {
    delete storage;
    storage = new Storage(chunks);
    for (int i = 0; i < chunks; i++) {
        buffers[i] = i + MULTIDRAW_BASE;
    }
}
HOOK(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers)) {
    if (buffers[0] >= MULTIDRAW_BASE) {
        delete storage;
    } else {
        ensure_glDeleteBuffers();
        real_glDeleteBuffers(n, buffers);
    }
}

// Setup Fake OpenGL Buffers
static int current_chunk = -1;
HOOK(glBindBuffer, void, (const GLenum target, GLuint buffer)) {
    if (target == GL_ARRAY_BUFFER && buffer >= MULTIDRAW_BASE && storage != nullptr) {
        current_chunk = int(buffer - MULTIDRAW_BASE);
        buffer = storage->buffer->server_side_data;
    } else {
        current_chunk = -1;
    }
    ensure_glBindBuffer();
    real_glBindBuffer(target, buffer);
}
HOOK(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage)) {
    if (target == GL_ARRAY_BUFFER && current_chunk >= 0 && storage != nullptr) {
        storage->upload(current_chunk, size, data);
    } else {
        ensure_glBufferData();
        real_glBufferData(target, size, data, usage);
    }
}

// Render
#define VERTEX_SIZE 24
#define MAX_RENDER_CHUNKS 4096
static bool supports_multidraw() {
    static int ret = -1;
    if (ret == -1) {
        ret = media_has_extension("GL_EXT_multi_draw_arrays");
    }
    return ret;
}
static GLint multidraw_firsts[MAX_RENDER_CHUNKS];
static GLsizei multidraw_counts[MAX_RENDER_CHUNKS];
static GLsizei multidraw_total = 0;
static void multidraw_renderSameAsLast(const LevelRenderer *self, const float b) {
    // Prepare Offset
    const Mob *camera = self->minecraft->camera;
    const float x = camera->old_x + ((camera->x - camera->old_x) * b);
    const float y = camera->old_y + ((camera->y - camera->old_y) * b);
    const float z = camera->old_z + ((camera->z - camera->old_z) * b);
    glPushMatrix();
    glTranslatef(-x, -y, -z);

    // Setup OpenGL
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, storage->buffer->server_side_data);
    glVertexPointer(3, GL_FLOAT, VERTEX_SIZE, (void *) 0);
    glTexCoordPointer(2, GL_FLOAT, VERTEX_SIZE, (void *) 0xc);
    glColorPointer(4, GL_UNSIGNED_BYTE, VERTEX_SIZE, (void *) 0x14);

    // Draw
#ifdef MCPI_USE_GLES1_COMPATIBILITY_LAYER
    if (supports_multidraw()) {
        glMultiDrawArrays(GL_TRIANGLES, multidraw_firsts, multidraw_counts, multidraw_total);
    } else {
#endif
        for (int i = 0; i < multidraw_total; i++) {
            glDrawArrays(GL_TRIANGLES, multidraw_firsts[i], multidraw_counts[i]);
        }
#ifdef MCPI_USE_GLES1_COMPATIBILITY_LAYER
    }
#endif

    // Cleanup
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
}
static int LevelRenderer_renderChunks_injection(__attribute__((unused)) LevelRenderer_renderChunks_t original, LevelRenderer *self, const int start, const int end, const int a, const float b) {
    // Batch
    multidraw_total = 0;
    for (int i = start; i < end; i++) {
        Chunk *chunk = self->chunks[i];
        // Check If Chunk Is Visible
        if (!chunk->field_1c[a] && chunk->visible) {
            const RenderChunk *render_chunk = chunk->getRenderChunk(a);
            // Get Data Block
            const int chunk_id = int(render_chunk->buffer - MULTIDRAW_BASE);
            const Block *block = storage->chunk_to_block[chunk_id];
            if (block == nullptr) {
                continue;
            }
            // Queue
            const int j = multidraw_total++;
            multidraw_firsts[j] = block->offset / VERTEX_SIZE;
            multidraw_counts[j] = render_chunk->vertices;
        }
    }

    // Draw
    multidraw_renderSameAsLast(self, b);

    // Return
    return multidraw_total;
}

// API
static bool use_multidraw = false;
void LevelRenderer_renderSameAsLast(LevelRenderer *self, const float delta) {
    if (use_multidraw) {
        multidraw_renderSameAsLast(self, delta);
    } else {
        self->render_list.render();
    }
}

// Init
void init_multidraw() {
    // Setup
    if (feature_has("Multidraw Rendering", server_disabled)) {
        overwrite_call((void *) 0x4e51c, (void *) setup_multidraw);
        overwrite_call((void *) 0x4e6f8, (void *) setup_multidraw);
        overwrite_calls(LevelRenderer_renderChunks, LevelRenderer_renderChunks_injection);
        unsigned char nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
        patch((void *) 0x479fc, nop_patch);
        use_multidraw = true;
    }
}