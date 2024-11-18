#include <libreborn/string.h>

// Sanitize String
void sanitize_string(std::string &str, const int max_length, const bool allow_newlines) {
    // Store Message Length
    size_t length = str.size();
    // Truncate Message
    if (max_length >= 0 && length > ((size_t) max_length)) {
        str = str.substr(0, max_length);
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