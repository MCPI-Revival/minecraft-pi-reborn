#include <libreborn/libreborn.h>

#include <mods/text-input-box/TextInputBox.h>

TextInputBox TextInputBox::create(int id, const std::string &placeholder, const std::string &text) {
    // Construct
    TextInputBox self;
    GuiComponent_constructor(&self.super);

    // Setup
    self.m_ID = id;
    self.m_xPos = 0;
    self.m_yPos = 0;
    self.m_width = 0;
    self.m_height = 0;
    self.m_placeholder = placeholder;
    self.m_text = text;
    self.m_bFocused = false;
    self.m_bEnabled = true;
    self.m_bCursorOn = true;
    self.m_insertHead = 0;
    self.m_lastFlashed = 0;
    self.m_pFont = nullptr;
    self.m_maxLength = -1;

    // Return
    return self;
}

void TextInputBox::setSize(int x, int y, int width, int height) {
    m_xPos = x;
    m_yPos = y;
    m_width = width;
    m_height = height;
}

void TextInputBox::init(Font *pFont) {
    m_pFont = pFont;
}

void TextInputBox::setEnabled(bool bEnabled) {
    m_bEnabled = bEnabled;
}

void TextInputBox::keyPressed(int key) {
    if (!m_bFocused) {
        return;
    }

    switch (key) {
        case 0x8: {
            // Backspace
            if (m_text.empty()) {
                return;
            }
            if (m_insertHead <= 0) {
                return;
            }
            if (m_insertHead > int(m_text.size())) {
                m_insertHead = int(m_text.size());
            }
            m_text.erase(m_text.begin() + m_insertHead - 1, m_text.begin() + m_insertHead);
            m_insertHead--;
            break;
        }
        case 0x2e: {
            // Delete
            if (m_text.empty()) {
                return;
            }
            if (m_insertHead < 0) {
                return;
            }
            if (m_insertHead >= int(m_text.size())) {
                return;
            }
            m_text.erase(m_text.begin() + m_insertHead, m_text.begin() + m_insertHead + 1);
            break;
        }
        case 0x25: {
            // Left
            m_insertHead--;
            if (m_insertHead < 0) {
                m_insertHead = 0;
            }
            break;
        }
        case 0x27: {
            // Right
            m_insertHead++;
            if (!m_text.empty()) {
                if (m_insertHead > int(m_text.size())) {
                    m_insertHead = int(m_text.size());
                }
            } else {
                m_insertHead = 0;
            }
            break;
        }
        case 0x0d: {
            // Enter
            m_bFocused = false;
            break;
        }
    }
}

void TextInputBox::tick() {
    if (!m_lastFlashed) {
        m_lastFlashed = Common_getTimeMs();
    }

    if (m_bFocused) {
        if (Common_getTimeMs() > m_lastFlashed + 500) {
            m_lastFlashed += 500;
            m_bCursorOn ^= 1;
        }
    } else {
        m_bCursorOn = false;
    }
}

void TextInputBox::setFocused(bool b) {
    if (m_bFocused == b) {
        return;
    }

    m_bFocused = b;
    if (b) {
        m_lastFlashed = Common_getTimeMs();
        m_bCursorOn = true;
        m_insertHead = int(m_text.size());
    }
}

void TextInputBox::onClick(int x, int y) {
    setFocused(clicked(x, y));
}

static int PADDING = 5;
void TextInputBox::charPressed(int k) {
    if (!m_bFocused) {
        return;
    }

    // note: the width will increase by the same amount no matter where K is appended
    std::string test_str = m_text + char(k);
    if (m_maxLength != -1 && int(test_str.length()) > m_maxLength) {
        return;
    }
    int width = Font_width(m_pFont, &test_str);
    if (width < (m_width - PADDING)) {
        m_text.insert(m_text.begin() + m_insertHead, k);
        m_insertHead++;
    }
}

void TextInputBox::render() {
    GuiComponent_fill(&super, m_xPos, m_yPos, m_xPos + m_width, m_yPos + m_height, 0xFFAAAAAA);
    GuiComponent_fill(&super, m_xPos + 1, m_yPos + 1, m_xPos + m_width - 1, m_yPos + m_height - 1, 0xFF000000);

    int textYPos = (m_height - 8) / 2;

    if (m_text.empty()) {
        GuiComponent_drawString(&super, m_pFont, &m_placeholder, m_xPos + PADDING, m_yPos + textYPos, 0x404040);
    } else {
        GuiComponent_drawString(&super, m_pFont, &m_text, m_xPos + PADDING, m_yPos + textYPos, 0xFFFFFF);
    }

    if (m_bCursorOn) {
        int xPos = 5;

        std::string substr = m_text.substr(0, m_insertHead);
        xPos += Font_width(m_pFont, &substr);

        std::string str = "_";
        GuiComponent_drawString(&super, m_pFont, &str, m_xPos + xPos, m_yPos + textYPos + 2, 0xFFFFFF);
    }
}

bool TextInputBox::clicked(int xPos, int yPos) {
    if (!m_bEnabled) {
        return false;
    }

    if (xPos < m_xPos) {
        return false;
    }
    if (yPos < m_yPos) {
        return false;
    }
    if (xPos >= m_xPos + m_width) {
        return false;
    }
    if (yPos >= m_yPos + m_height) {
        return false;
    }

    return true;
}
