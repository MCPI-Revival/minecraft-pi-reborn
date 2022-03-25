#include <libreborn/string.h>

// Sanitize String
#define MINIMUM_SAFE_CHARACTER 32
#define MAXIMUM_SAFE_CHARACTER 126
void sanitize_string(char **str, int max_length, unsigned int allow_newlines) {
    // Store Message Length
    int length = strlen(*str);
    // Truncate Message
    if (max_length != -1 && length > max_length) {
        (*str)[max_length] = '\0';
        length = max_length;
    }
    // Loop Through Message
    for (int i = 0; i < length; i++) {
        if (allow_newlines && ((*str)[i] == '\n' || (*str)[i] == '\r')) {
            continue;
        }
        unsigned char c = (unsigned char) (*str)[i];
        if (c < MINIMUM_SAFE_CHARACTER || c > MAXIMUM_SAFE_CHARACTER) {
            // Replace Illegal Character
            (*str)[i] = '?';
        }
    }
}

// Starts With
int starts_with(const char *str, const char *prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}
