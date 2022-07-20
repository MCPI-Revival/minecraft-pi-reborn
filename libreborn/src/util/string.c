#include <iconv.h>
#include <stdint.h>

#include <libreborn/string.h>

// Sanitize String
void sanitize_string(char **str, int max_length, unsigned int allow_newlines) {
    // Store Message Length
    int length = strlen(*str);
    // Truncate Message
    if (max_length != -1 && length > max_length) {
        (*str)[max_length] = '\0';
        length = max_length;
    }
    // Loop Through Message
    if (!allow_newlines) {
        for (int i = 0; i < length; i++) {
            if ((*str)[i] == '\n' || (*str)[i] == '\r') {
                // Replace Newline
                (*str)[i] = ' ';
            }
        }
    }
}

// Minecraft-Flavored CP437
void safe_iconv(iconv_t cd, char *input, size_t input_size, char *output, size_t output_size) {
    iconv(cd, &input, &input_size, &output, &output_size);
}
#define CP437_CHARACTERS 256
static const char *cp437_characters_map[CP437_CHARACTERS] = {
    "\0", "☺", "☻", "♥", "♦", "♣", "♠", "•", "◘", "○", "\n", "♂", "♀", "\r", "♫", "☼",
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
        iconv_t cd = iconv_open("UTF-32LE", "UTF-8");
        if (cd != (iconv_t) -1) {
            size_t str_size = 4;
            uint32_t *str = (uint32_t *) malloc(str_size);
            ALLOC_CHECK(str);
            for (int i = 0; i < CP437_CHARACTERS; i++) {
                // Convert to UTF-32, Then Extract Codepoint
                safe_iconv(cd, (char *) cp437_characters_map[i], strlen(cp437_characters_map[i]), (char *) str, str_size);
                // Extract
                map[i] = str[0];
            }
            // Free
            free(str);
            iconv_close(cd);
        } else {
            IMPOSSIBLE();
        }
        is_setup = 1;
    }
    return map;
}
char *to_cp437(const char *input) {
    // Convert To UTF-32 For Easier Parsing
    size_t in_size = strlen(input);
    size_t utf32_str_size = in_size * 4;
    size_t real_utf32_str_size = utf32_str_size + 4 /* NULL-terminator */;
    uint32_t *utf32_str = (uint32_t *) malloc(real_utf32_str_size);
    ALLOC_CHECK(utf32_str);
    memset(utf32_str, 0, real_utf32_str_size);
    iconv_t cd = iconv_open("UTF-32LE", "UTF-8");
    if (cd != (iconv_t) -1) {
        safe_iconv(cd, (char *) input, in_size, (char *) utf32_str, utf32_str_size);
        iconv_close(cd);
    } else {
        IMPOSSIBLE();
    }
    // Allocate String
    size_t cp437_str_size;
    for (cp437_str_size = 0; utf32_str[cp437_str_size] != 0; cp437_str_size++);
    size_t real_cp437_str_size = cp437_str_size + 1 /* NULL-terminator */;
    char *cp437_str = (char *) malloc(real_cp437_str_size);
    ALLOC_CHECK(cp437_str);
    memset(cp437_str, 0, real_cp437_str_size);
    // Handle Characters
    for (size_t i = 0; utf32_str[i] != 0; i++) {
        uint32_t codepoint = utf32_str[i];
        for (int j = 0; j < CP437_CHARACTERS; j++) {
            uint32_t test_codepoint = get_cp437_characters_codepoint_map()[j];
            if (codepoint == test_codepoint) {
                cp437_str[i] = j;
                break;
            }
        }
        if (cp437_str[i] == '\0') {
            cp437_str[i] = '?';
        }
    }
    // Free
    free(utf32_str);
    // Return
    return cp437_str;
}
char *from_cp437(const char *input) {
    // Convert To UTF-32 For Easier Parsing
    size_t in_size = strlen(input);
    size_t utf32_str_size = in_size * 4;
    size_t real_utf32_str_size = utf32_str_size + 4 /* NULL-terminator */;
    uint32_t *utf32_str = (uint32_t *) malloc(real_utf32_str_size);
    ALLOC_CHECK(utf32_str);
    memset(utf32_str, 0, real_utf32_str_size);
    // Handle Characters
    for (size_t i = 0; input[i] != '\0'; i++) {
        utf32_str[i] = get_cp437_characters_codepoint_map()[(uint32_t) input[i]];
    }
    // Convert To UTF-8
    size_t out_size = utf32_str_size;
    size_t real_out_size = utf32_str_size + 1 /* NULL-terminator */;
    char *output = (char *) malloc(real_out_size);
    ALLOC_CHECK(output);
    memset(output, 0, real_out_size);
    iconv_t cd = iconv_open("UTF-8", "UTF-32LE");
    if (cd != (iconv_t) -1) {
        safe_iconv(cd, (char *) utf32_str, utf32_str_size, output, out_size);
        iconv_close(cd);
    } else {
        IMPOSSIBLE();
    }
    // Return
    return output;
}

// Starts With
int starts_with(const char *str, const char *prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}
