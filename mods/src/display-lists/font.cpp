#include <libreborn/patch.h>

#include <symbols/Font.h>
#include <symbols/Tesselator.h>
#include <symbols/Textures.h>

#include <GLES/gl.h>

#include "internal.h"

// Create Display lists
static void Font_init_injection(Font_init_t original, Font *self, Options *options) {
    // Call Original Method
    original(self, options);

    // Generate Lists
    constexpr int character_count = sizeof(self->character_widths) / sizeof(int);
    self->display_lists = media_glGenLists(character_count);
    for (int i = 0; i < character_count; i++) {
        media_glNewList(self->display_lists + i, GL_COMPILE);
        Tesselator &t = Tesselator::instance;
        t.begin(GL_QUADS);
        self->buildChar(uchar(i), 0, 0);
        t.draw();
        media_glTranslatef(GLfloat(self->character_widths[i]), 0, 0);
        media_glEndList();
    }
}

// Custom Rendering
static void Font_drawSlow_injection(MCPI_UNUSED Font_drawSlow_t original, Font *self, const char *text, float x, float y, uint color, bool param_1) {
    // Check Text
    if (!text) {
        return;
    }
    const size_t text_length = strlen(text);
    if (text_length == 0) {
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

    // Build Text
    std::vector<int> list;
    list.reserve(text_length);
    for (size_t i = 0; i < text_length; i++) {
        list.push_back(self->display_lists + uchar(text[i]));
    }

    // Render
    media_glPushMatrix();
    media_glTranslatef(x, y, 0);
    media_glCallLists(list.size(), GL_UNSIGNED_INT, list.data());
    media_glPopMatrix();

    // Reset Color
    media_glColor4f(1, 1, 1, 1);
}

// Init
void _init_display_lists_font() {
    overwrite_calls(Font_init, Font_init_injection);
    overwrite_calls(Font_drawSlow, Font_drawSlow_injection);
}