#include <libreborn/string.h>

#include <string.h>

// Sanitize String
void sanitize_string(char *str, const int max_length, const int allow_newlines) {
    // Store Message Length
    size_t length = strlen(str);
    // Truncate Message
    if (max_length >= 0 && length > ((size_t) max_length)) {
        str[max_length] = '\0';
        length = max_length;
    }
    // Loop Through Message
    if (!allow_newlines) {
        for (size_t i = 0; i < length; i++) {
            if (str[i] == '\n') {
                // Replace Newline
                str[i] = ' ';
            }
        }
    }
}

// Starts With
int starts_with(const char *str, const char *prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}
