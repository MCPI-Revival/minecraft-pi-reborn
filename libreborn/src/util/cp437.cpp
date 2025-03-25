#include <unordered_map>

#include "utf8.h"

#include <libreborn/util/string.h>

// Minecraft-Flavored CP-437
#define CP437_CHARACTERS 256
static const std::string cp437_characters_map[CP437_CHARACTERS] = {
    "\0", "☺", "☻", "♥", "♦", "♣", "♠", "•", "◘", "○", "\n", "♂", "♀", "♪", "♫", "☼",
    "►", "◄", "↕", "‼", "¶", "§", "▬", "↨", "↑", "↓", "→", "←", "∟", "↔", "▲", "▼",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "⌂",
    "Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å",
    "É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "¢", "£", "¥", "₧", "ƒ",
    "á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "⌐", "¬", "½", "¼", "¡", "«", "»",
    "░", "▒", "▓", "│", "┤", "╡", "╢", "╖", "╕", "╣", "║", "╗", "╝", "╜", "╛", "┐",
    "└", "┴", "┬", "├", "─", "┼", "╞", "╟", "╚", "╔", "╩", "╦", "╠", "═", "╬", "╧",
    "╨", "╤", "╥", "╙", "╘", "╒", "╓", "╫", "╪", "┘", "┌", "█", "▄", "▌", "▐", "▀",
    "α", "ß", "Γ", "π", "Σ", "σ", "µ", "τ", "Φ", "Θ", "Ω", "δ", "∞", "φ", "ε", "∩",
    "≡", "±", "≥", "≤", "⌠", "⌡", "÷", "≈", "°", "∙", "·", "√", "ⁿ", "²", "■", "©"
};

// Character Conversion Functions
static char32_t cp437_to_utf32(const unsigned char c) {
    // Map
    static char32_t map[CP437_CHARACTERS];
    static bool is_setup = false;
    if (!is_setup) {
        // Build Map
        for (int i = 0; i < CP437_CHARACTERS; i++) {
            map[i] = utf8::unchecked::peek_next(cp437_characters_map[i].begin());
        }
        is_setup = true;
    }
    // Convert
    return map[int(c)];
}
unsigned char utf32_to_cp437(const char32_t codepoint) {
    // Map
    static std::unordered_map<char32_t, unsigned char> map;
    if (map.empty()) {
        // Build Map
        for (int i = 0; i < CP437_CHARACTERS; i++) {
            const unsigned char c = (unsigned char) i;
            map[cp437_to_utf32(c)] = c;
        }
    }
    // Convert
    if (map.contains(codepoint)) {
        return map.at(codepoint);
    } else {
        // Invalid Character
        return '?';
    }
}

// String Conversion Functions
std::string to_cp437(const std::string &input) {
    // Allocate String
    std::string cp437_str;
    // CP-437 String Will Never Be Longer Than The Equivalent UTF-8 String
    cp437_str.reserve(input.length());

    // Handle Characters
    std::string::const_iterator it = input.begin();
    while (it != input.end()) {
        const char32_t codepoint = utf8::unchecked::next(it);
        // Find Corresponding CP-437 Character
        const unsigned char x = utf32_to_cp437(codepoint);
        // Add To String
        cp437_str.push_back(x);
    }

    // Return
    return cp437_str;
}
std::string from_cp437(const std::string &input) {
    // Allocate String
    std::string out;
    // UTF-8 String May Be Longer Than The Equivalent CP-437 String
    // Assume The Worst
    out.reserve(input.length() * 4);

    // Handle Characters
    std::back_insert_iterator<std::string> it = std::back_inserter(out);
    for (const char c : input) {
        // Get Unicode Codepoint
        const char32_t codepoint = cp437_to_utf32((unsigned char) c);
        // Append To String
        it = utf8::unchecked::append(codepoint, it);
    }

    // Return
    return out;
}
