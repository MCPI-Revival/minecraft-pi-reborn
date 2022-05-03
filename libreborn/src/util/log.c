#include <stdio.h>
#include <unistd.h>

#define COLOR(name, value) \
    char *color_##name() { \
        static char *out = NULL; \
        if (out == NULL) { \
            out = isatty(fileno(stderr)) ? "\x1b[" value "m" : ""; \
        } \
        return out; \
    }

COLOR(reset, "0")
COLOR(yellow, "93")
COLOR(faint, "2")
COLOR(red, "91")
