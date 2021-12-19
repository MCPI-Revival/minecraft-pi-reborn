#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

// Set obj To NULL On asprintf() Failure
#define safe_asprintf(obj, ...) \
    { \
        if (asprintf(obj, __VA_ARGS__) == -1) { \
            *obj = NULL; \
        } \
        ALLOC_CHECK(*obj); \
    }

// Dynamic String Append Macro
#define string_append(str, format, ...) \
    { \
        char *old = *str; \
        safe_asprintf(str, "%s" format, *str == NULL ? "" : *str, __VA_ARGS__); \
        ALLOC_CHECK(*str); \
        if (old != NULL && old != *str) { \
            free(old); \
        } \
    }

// Sanitize String
#define MINIMUM_SAFE_CHARACTER 32
#define MAXIMUM_SAFE_CHARACTER 126
static inline void sanitize_string(char **str, int max_length, unsigned int allow_newlines) {
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
