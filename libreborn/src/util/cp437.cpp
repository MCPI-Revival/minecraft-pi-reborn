#include <cstdint>
#include <cstdlib>

#include "utf8.h"

#include <libreborn/string.h>

// Conversion Functions
static std::u32string to_utf32(const std::string &s) {
    return utf8::utf8to32(s);
}
static std::string to_utf8(const std::u32string &s) {
    return utf8::utf32to8(s);
}

// Minecraft-Flavored CP437
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
static uint32_t *get_cp437_characters_codepoint_map() {
    static uint32_t map[CP437_CHARACTERS];
    static int is_setup = 0;
    if (!is_setup) {
        // Build Map
        for (int i = 0; i < CP437_CHARACTERS; i++) {
            // Convert to UTF-32, Then Extract Codepoint
            std::u32string str = to_utf32(cp437_characters_map[i]);
            // Extract
            map[i] = str[0];
        }
        is_setup = 1;
    }
    return map;
}
char *to_cp437(const char *input) {
    // Convert To UTF-32 For Easier Parsing
    std::u32string utf32_str = to_utf32(input);

    // Allocate String
    std::string cp437_str;

    // Handle Characters
    for (size_t i = 0; i < utf32_str.length(); i++) {
        uint32_t codepoint = utf32_str[i];
        bool valid = false;
        for (int j = 0; j < CP437_CHARACTERS; j++) {
            uint32_t test_codepoint = get_cp437_characters_codepoint_map()[j];
            if (codepoint == test_codepoint) {
                valid = true;
                cp437_str += j;
                break;
            }
        }
        if (!valid) {
            cp437_str += '?';
        }
    }

    // Return
    return strdup(cp437_str.c_str());
}
char *from_cp437(const char *raw_input) {
    // Convert To UTF-32 For Easier Parsing
    std::string input = raw_input;
    std::u32string utf32_str;

    // Handle Characters
    for (size_t i = 0; i < input.length(); i++) {
        unsigned char c = (unsigned char) input[i];
        utf32_str += get_cp437_characters_codepoint_map()[(uint32_t) c];
    }

    // Convert To UTF-8
    return strdup(to_utf8(utf32_str).c_str());
}
