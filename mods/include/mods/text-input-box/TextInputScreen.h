#pragma once

#include <symbols/minecraft.h>

#include <mods/text-input-box/TextInputBox.h>

struct TextInputScreen {
    std::vector<TextInputBox *> *m_textInputs = nullptr;

    template <typename T>
    static void setup(Screen_vtable *vtable) {
#define PATCH_VTABLE(name) \
    static Screen_##name##_t original_##name = vtable->name; \
    vtable->name = [](Screen *super, auto... args) { \
        original_##name(super, std::forward<decltype(args)>(args)...); \
        T *self = (T *) super; \
        self->data.text_input.name(std::forward<decltype(args)>(args)...); \
    }
        PATCH_VTABLE(keyPressed);
        PATCH_VTABLE(keyboardNewChar);
        PATCH_VTABLE(mouseClicked);
        PATCH_VTABLE(render);
        PATCH_VTABLE(init);
        PATCH_VTABLE(removed);
#undef PATCH_VTABLE
    }

private:
    void keyPressed(int key) const;
    void keyboardNewChar(char key) const;
    void mouseClicked(int x, int y, int param_1) const;
    void render(int x, int y, float param_1) const;
    void init();
    void removed() const;
};
