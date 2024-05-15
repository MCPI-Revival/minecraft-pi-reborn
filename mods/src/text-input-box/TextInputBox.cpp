#include <libreborn/libreborn.h>

#include <mods/text-input-box/TextInputBox.h>

TextInputBox *TextInputBox::create(const std::string &placeholder, const std::string &text) {
    // Construct
    TextInputBox *self = new TextInputBox;
    self->super.constructor();

    // Setup
    self->m_xPos = 0;
    self->m_yPos = 0;
    self->m_width = 0;
    self->m_height = 0;
    self->m_placeholder = placeholder;
    self->m_text = text;
    self->m_bFocused = false;
    self->m_bEnabled = true;
    self->m_bCursorOn = true;
    self->m_insertHead = 0;
    self->m_lastFlashed = 0;
    self->m_pFont = nullptr;
    self->m_maxLength = -1;
    self->m_scrollPos = 0;

    // Return
    return self;
}

void TextInputBox::setSize(int x, int y, int width, int height) {
    m_xPos = x;
    m_yPos = y;
    m_width = width;
    m_height = height;
    recalculateScroll();
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
            recalculateScroll();
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
            recalculateScroll();
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
            recalculateScroll();
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
        m_lastFlashed = Common::getTimeMs();
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
        m_lastFlashed = Common::getTimeMs();
        m_bCursorOn = true;
        m_insertHead = int(m_text.size());
        recalculateScroll();
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

    // Ignore Newlines
    if (k == '\n') {
        return;
    }

    // Check Max Length
    if (m_maxLength != -1 && int(m_text.length()) >= m_maxLength) {
        return;
    }

    // Insert
    m_text.insert(m_text.begin() + m_insertHead, k);
    m_insertHead++;
    recalculateScroll();
}

static std::string get_rendered_text(Font *font, int width, int scroll_pos, std::string text) {
    std::string rendered_text = text.substr(scroll_pos);
    int max_width = width - (PADDING * 2);
    while (Font_width(font, &rendered_text) > max_width) {
        rendered_text.pop_back();
    }
    return rendered_text;
}

static char CURSOR_CHAR = '_';

void TextInputBox::render() {
    super.fill(m_xPos, m_yPos, m_xPos + m_width, m_yPos + m_height, 0xFFAAAAAA);
    super.fill(m_xPos + 1, m_yPos + 1, m_xPos + m_width - 1, m_yPos + m_height - 1, 0xFF000000);

    int text_color;
    int scroll_pos;
    std::string rendered_text;
    if (m_text.empty()) {
        rendered_text = m_placeholder;
        text_color = 0x404040;
        scroll_pos = 0;
    } else {
        rendered_text = m_text;
        text_color = 0xffffff;
        scroll_pos = m_scrollPos;
    }
    rendered_text = get_rendered_text(m_pFont, m_width, scroll_pos, rendered_text);

    int textYPos = (m_height - 8) / 2;
    super.drawString(m_pFont, &rendered_text, m_xPos + PADDING, m_yPos + textYPos, text_color);

    if (m_bCursorOn) {
        int cursor_pos = m_insertHead - m_scrollPos;
        if (cursor_pos >= 0 && cursor_pos <= int(rendered_text.length())) {
            std::string substr = rendered_text.substr(0, cursor_pos);
            int xPos = PADDING + m_pFont->width(&substr);

            std::string str;
            str += CURSOR_CHAR;
            super.drawString(m_pFont, &str, m_xPos + xPos, m_yPos + textYPos + 2, 0xffffff);
        }
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

void TextInputBox::recalculateScroll() {
    // Skip If Size Unset
    if (m_width == 0) {
        return;
    }
    // Ensure Cursor Is Visible
    bool is_cursor_at_end = m_insertHead == int(m_text.length());
    if (m_scrollPos >= m_insertHead && m_scrollPos > 0) {
        // Cursor Is At Scroll Position
        // Move Back Scroll As Far As Possible
        while (true) {
            int test_scroll_pos = m_scrollPos - 1;
            std::string rendered_text = m_text;
            if (is_cursor_at_end) {
                rendered_text += CURSOR_CHAR;
            }
            rendered_text = get_rendered_text(m_pFont, m_width, test_scroll_pos, rendered_text);
            int cursor_pos = m_insertHead - test_scroll_pos;
            if (cursor_pos >= int(rendered_text.length())) {
                break;
            } else {
                m_scrollPos = test_scroll_pos;
                if (m_scrollPos == 0) {
                    break;
                }
            }
        }
    } else {
        // Cursor After Scroll Area
        // Increase Scroll So Cursor Is Visible
        while (true) {
            std::string rendered_text = m_text;
            if (is_cursor_at_end) {
                rendered_text += CURSOR_CHAR;
            }
            rendered_text = get_rendered_text(m_pFont, m_width, m_scrollPos, rendered_text);
            int cursor_pos = m_insertHead - m_scrollPos;
            if (cursor_pos < int(rendered_text.length())) {
                break;
            } else {
                if (m_scrollPos == int(m_text.length())) {
                    WARN("Text Box Is Too Small");
                    break;
                } else {
                    m_scrollPos++;
                }
            }
        }
    }
}

std::string TextInputBox::getText() {
    return m_text;
}

void TextInputBox::setText(std::string str) {
    m_text = str;
    m_insertHead = int(m_text.size());
}

bool TextInputBox::isFocused() {
    return m_bFocused;
}

void TextInputBox::setMaxLength(int max_length) {
    m_maxLength = max_length;
}

