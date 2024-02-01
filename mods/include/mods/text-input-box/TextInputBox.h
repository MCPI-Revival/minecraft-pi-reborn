#pragma once

#include <symbols/minecraft.h>

struct TextInputBox {
    GuiComponent super;

    int m_ID;
    int m_xPos;
    int m_yPos;
    int m_width;
    int m_height;
    std::string m_placeholder;
    std::string m_text;
    bool m_bFocused;
    bool m_bEnabled;
    bool m_bCursorOn;
    int m_insertHead;
    int m_lastFlashed;
    Font *m_pFont;
    int m_maxLength;

    void setSize(int x, int y, int width = 200, int height = 12);
    void init(Font *pFont);
    void setEnabled(bool bEnabled);
    void keyPressed(int key);
    void charPressed(int chr);
    void render();
    void tick();
    void setFocused(bool b);
    void onClick(int x, int y);
    bool clicked(int x, int y);

    static TextInputBox *create(int id, const std::string &placeholder = "", const std::string &text = "");
};
