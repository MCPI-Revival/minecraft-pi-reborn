#include <libreborn/patch.h>
#include <libreborn/util/util.h>

#include <symbols/Tesselator.h>
#include <symbols/Common.h>
#include <symbols/ModelPart.h>
#include <symbols/Cube.h>
#include <symbols/Minecraft.h>

#include <GLES/gl.h>

#include <mods/tesselator/tesselator.h>

#include "internal.h"

// Map Buffers To Display Lists
static std::unordered_map<GLuint, GLuint> buffer_to_display_list;

// Draw To Buffer
unsigned int _display_lists_get_for_buffer(const unsigned int buffer) {
    return buffer_to_display_list.at(buffer);
}
__attribute__((hot)) static RenderChunk Tesselator_end_injection(Tesselator *self, const bool use_given_buffer, const int buffer) {
    // Check
    const CustomTesselator &t = advanced_tesselator_get();
    RenderChunk out = {};
    out.constructor();
    if (t.void_begin_end) {
        return out;
    }

    // Get Display List
    if (!use_given_buffer) {
        IMPOSSIBLE();
    }
    if (!buffer_to_display_list.contains(buffer)) {
        buffer_to_display_list.insert({buffer, media_glGenLists(1)});
    }
    const GLuint list = _display_lists_get_for_buffer(buffer);

    // Render
    media_glNewList(list, GL_COMPILE);
    self->draw();
    media_glEndList();

    // Finish
    out.buffer = buffer;
    return out;
}

// Draw Buffer
__attribute__((hot)) static void Common_drawArrayVT_injection(const int buffer, MCPI_UNUSED const int vertices, MCPI_UNUSED int vertex_size, MCPI_UNUSED uint mode) {
    // These Are Really Display Lists
    const GLuint list = _display_lists_get_for_buffer(buffer);
    media_glCallLists(1, GL_UNSIGNED_INT, &list);
}

// Fix Model Rendering
static void ModelPart_compile_injection(MCPI_UNUSED ModelPart_compile_t original, ModelPart *self, const float scale) {
    Tesselator &t = Tesselator::instance;
    t.begin(GL_QUADS);
    for (Cube *cube : self->cubes) {
        cube->compile(t, scale);
    }
    t.end(true, int(self->buffer));
    self->compiled = true;
}

// Fix Items Being Too Bright
static void ItemInHandRenderer_renderItem_Tesselator_begin_injection(Tesselator *self) {
    // Call Original Method
    self->begin_quads();
    // Prevent Color Data From Being Included
    self->noColor();
}

// Properly Delete Display Lists
HOOK(media_glDeleteBuffers, void, (GLsizei n, const GLuint *buffers)) {
    real_media_glDeleteBuffers()(n, buffers);
    for (int i = 0; i < n; i++) {
        const GLuint buffer = buffers[i];
        if (buffer_to_display_list.contains(buffer)) {
            media_glDeleteLists(_display_lists_get_for_buffer(buffer), 1);
            buffer_to_display_list.erase(buffer);
        }
    }
}

// Handle Graphics Reset
static void Minecraft_onGraphicsReset_injection(Minecraft_onGraphicsReset_t original, Minecraft *self) {
    // Clear Map
    buffer_to_display_list.clear();
    // Call Original Method
    original(self);
}

// Init
void _init_display_lists_tesselator() {
    advanced_tesselator_enable();
    ignore_patch_conflict = true;
    replace_func(Tesselator_end);
    replace_func(Common_drawArrayVT);
    ignore_patch_conflict = false;
    overwrite_calls(ModelPart_compile, ModelPart_compile_injection);
    overwrite_call((void *) 0x4b9a0, Tesselator_begin_quads, ItemInHandRenderer_renderItem_Tesselator_begin_injection);
    overwrite_call((void *) 0x4babc, Tesselator_begin_quads, ItemInHandRenderer_renderItem_Tesselator_begin_injection);
    overwrite_calls(Minecraft_onGraphicsReset, Minecraft_onGraphicsReset_injection);
}