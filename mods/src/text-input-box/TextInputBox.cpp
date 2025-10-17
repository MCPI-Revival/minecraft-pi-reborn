#include <libreborn/log.h>

#include <mods/text-input-box/TextInputBox.h>
#include <mods/input/input.h>
#include <mods/misc/misc.h>

// Constructor/Destructor
TextInputBox::TextInputBox(const std::string &placeholder, const std::string &text) {
    // Construct
    this->component = GuiComponent::allocate();
    this->component->constructor();

    // Setup
    this->m_xPos = 0;
    this->m_yPos = 0;
    this->m_width = 0;
    this->m_height = 0;
    this->m_placeholder = placeholder;
    this->m_text = text;
    this->m_bFocused = false;
    this->m_bEnabled = true;
    this->m_bCursorOn = true;
    this->m_insertHead = 0;
    this->m_lastFlashed = 0;
    this->m_pFont = nullptr;
    this->m_maxLength = -1;
    this->m_scrollPos = 0;
}
TextInputBox::~TextInputBox() {
    component->destructor_deleting();
}

// Set Dimensions
void TextInputBox::setSize(const int x, const int y, const int width, const int height) {
    m_xPos = x;
    m_yPos = y;
    m_width = width;
    m_height = height;
    recalculateScroll();
}

// Initialize
void TextInputBox::init(Font *pFont) {
    m_pFont = pFont;
}

// Enable/Disable
void TextInputBox::setEnabled(const bool bEnabled) {
    m_bEnabled = bEnabled;
}

// Handle Key Press
void TextInputBox::keyPressed(const int key) {
    if (!m_bFocused) {
        return;
    }

    switch (key) {
        case MC_KEY_BACKSPACE: {
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
        case MC_KEY_DELETE: {
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
        case MC_KEY_LEFT: {
            // Left
            m_insertHead--;
            if (m_insertHead < 0) {
                m_insertHead = 0;
            }
            recalculateScroll();
            break;
        }
        case MC_KEY_RIGHT: {
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
        case MC_KEY_RETURN: {
            // Enter
            m_bFocused = false;
            break;
        }
        default: {}
    }
}

// Tick
void TextInputBox::tick() {
    if (!m_lastFlashed) {
        m_lastFlashed = Common::getTimeMs();
    }

    if (m_bFocused) {
        if (Common::getTimeMs() > m_lastFlashed + 500) {
            m_lastFlashed += 500;
            m_bCursorOn ^= 1;
        }
    } else {
        m_bCursorOn = false;
    }
}

// Get/Set Focus
bool TextInputBox::isFocused() const {
    return m_bFocused;
}
void TextInputBox::setFocused(const bool b) {
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

// Handle Click Event
void TextInputBox::onClick(const int x, const int y) {
    setFocused(clicked(x, y));
}

// Handle Character Event
static int PADDING = 5;
void TextInputBox::charPressed(const int k) {
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

// Get Slice Of Text To Render
static std::string get_rendered_text(Font *font, const int width, const int scroll_pos, const std::string &text) {
    std::string rendered_text = text.substr(scroll_pos);
    const int max_width = width - (PADDING * 2);
    while (font->width(rendered_text) > max_width && !rendered_text.empty()) {
        rendered_text.pop_back();
    }
    return rendered_text;
}

// Render
static char CURSOR_CHAR = '_';
void TextInputBox::render() const {
    // Draw Background
    component->fill(m_xPos, m_yPos, m_xPos + m_width, m_yPos + m_height, 0xFFAAAAAA);
    component->fill(m_xPos + 1, m_yPos + 1, m_xPos + m_width - 1, m_yPos + m_height - 1, 0xFF000000);

    // Prepare
    constexpr int cursor_color = 0xe0e0e0;
    int text_color;
    int scroll_pos;
    std::string rendered_text;
    if (m_text.empty()) {
        rendered_text = m_placeholder;
        text_color = 0x404040;
        scroll_pos = 0;
    } else {
        rendered_text = m_text;
        text_color = m_bEnabled ? cursor_color : 0x707070;
        scroll_pos = m_scrollPos;
    }
    rendered_text = get_rendered_text(m_pFont, m_width, scroll_pos, rendered_text);

    // Draw Visible Text
    const int textYPos = (m_height - line_height) / 2;
    component->drawString(m_pFont, rendered_text, m_xPos + PADDING, m_yPos + textYPos, text_color);

    // Draw Cursor
    if (m_bCursorOn) {
        const int cursor_pos = m_insertHead - m_scrollPos;
        if (cursor_pos >= 0 && cursor_pos <= int(rendered_text.length())) {
            const std::string substr = rendered_text.substr(0, cursor_pos);
            const int xPos = PADDING + m_pFont->width(substr);

            std::string str;
            str += CURSOR_CHAR;
            component->drawString(m_pFont, str, m_xPos + xPos, m_yPos + textYPos + 2, cursor_color);
        }
    }
}

// Check If Mouse Event Clicked This Widget
bool TextInputBox::clicked(const int xPos, const int yPos) const {
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

// Calculate Scroll Position
void TextInputBox::recalculateScroll() {
    // Skip If Size Unset
    if (m_width == 0) {
        return;
    }
    // Ensure Cursor Is Visible
    const bool is_cursor_at_end = m_insertHead == int(m_text.length());
    if (m_scrollPos >= m_insertHead && m_scrollPos > 0) {
        // Cursor Is At Scroll Position
        // Move Back Scroll As Far As Possible
        while (true) {
            const int test_scroll_pos = m_scrollPos - 1;
            std::string rendered_text = m_text;
            if (is_cursor_at_end) {
                rendered_text += CURSOR_CHAR;
            }
            rendered_text = get_rendered_text(m_pFont, m_width, test_scroll_pos, rendered_text);
            const int cursor_pos = m_insertHead - test_scroll_pos;
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
            const int cursor_pos = m_insertHead - m_scrollPos;
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

// Get/Set Text
std::string TextInputBox::getText() const {
    return m_text;
}
void TextInputBox::setText(const std::string &text) {
    m_text = text;
    m_insertHead = int(m_text.size());
}

// Set Max Length
void TextInputBox::setMaxLength(const int max_length) {
    m_maxLength = max_length;
}
