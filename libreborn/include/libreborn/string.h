#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <stdint.h>

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
        safe_asprintf(str, "%s" format, *str == NULL ? "" : *str, ##__VA_ARGS__); \
        ALLOC_CHECK(*str); \
        if (old != NULL && old != *str) { \
            free(old); \
        } \
    }

#ifdef __cplusplus
extern "C" {
#endif

// Sanitize String
void sanitize_string(char **str, int max_length, unsigned int allow_newlines);

// CP437
void safe_iconv(iconv_t cd, char *input, size_t input_size, char *output, size_t output_size);
char *to_cp437(const char *input);
char *from_cp437(const char *input);

// Starts With
int starts_with(const char *str, const char *prefix);

#ifdef __cplusplus
}
#endif
