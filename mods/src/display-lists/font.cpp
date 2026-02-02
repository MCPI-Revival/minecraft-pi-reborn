#include <libreborn/patch.h>

#include <symbols/Font.h>
#include <symbols/Tesselator.h>
#include <symbols/Textures.h>

#include <GLES/gl.h>

#include <mods/tesselator/tesselator.h>

#include "internal.h"

// Create Display lists
static void Font_init_injection(Font_init_t original, Font *self, Options *options) {
    // Call Original Method
    original(self, options);

    // Generate Lists
    constexpr int character_count = sizeof(self->character_widths) / sizeof(int);
    self->display_lists = media_glGenLists(character_count);
    CustomTesselator &advanced_t = advanced_tesselator_get();
    advanced_t.are_vertices_flat = true;
    for (int i = 0; i < character_count; i++) {
        media_glNewList(self->display_lists + i, GL_COMPILE);
        Tesselator &t = Tesselator::instance;
        t.begin(GL_QUADS);
        self->buildChar(uchar(i), 0, 0);
        t.draw();
        media_glTranslatef(GLfloat(self->character_widths[i]), 0, 0);
        media_glEndList();
    }
    advanced_t.are_vertices_flat = false;
}

// Custom Rendering
static void Font_drawSlow_injection(Font_drawSlow_t original, Font *self, const char *text, const float x, float y, uint color, const bool param_1) {
    // If this is part of a batched render, do the old/slow method.
    const Tesselator &t = Tesselator::instance;
    if (t.void_begin_end) {
        original(self, text, x, y, color, param_1);
        return;
    }

    // Check Text
    if (!text || text[0] == '\0') {
        return;
    }

    // Darken Text
    if (param_1) {
        const uint val = color & 0xff000000;
        color = (color & 0xfcfcfc) >> 2;
        color += val;
    }

    // Bind Texture
    self->textures->loadAndBindTexture(self->texture_name);

    // Set Color
    float a = float((color >> 24) & 0xff) / 255.0f;
    if (a == 0) {
        a = 1;
    }
    const float r = float((color >> 16) & 0xff) / 255.0f;
    const float g = float((color >> 8) & 0xff) / 255.0f;
    const float b = float((color) & 0xff) / 255.0f;
    media_glColor4f(r, g, b, a);

    // Render Lines
    size_t position = 0;
    bool has_another_line = true;
    while (has_another_line) {
        // Handle Newline
        if (position > 0) {
            y += float(self->line_height);
        }
        has_another_line = false;

        // Read Line
        static constexpr int max_line_size = 512;
        static GLuint line[max_line_size];
        int line_size = 0;
        while (text[position] != '\0') {
            // Read Character
            const char c = text[position];
            position++;
            // Handle Character
            if (c == '\n') {
                // Found Newline
                has_another_line = true;
                break;
            } else if (line_size < max_line_size) {
                // Add To Render Command
                line[line_size] = self->display_lists + uchar(c);
                line_size++;
            }
        }

        // Render
        media_glPushMatrix();
        media_glTranslatef(x, y, 0);
        media_glCallLists(line_size, GL_UNSIGNED_INT, line);
        media_glPopMatrix();
    }

    // Reset Color
    media_glColor4f(1, 1, 1, 1);
}

// Init
void _init_display_lists_font() {
    advanced_tesselator_enable();
    overwrite_calls(Font_init, Font_init_injection);
    overwrite_calls(Font_drawSlow, Font_drawSlow_injection);
}