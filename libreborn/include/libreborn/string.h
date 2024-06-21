#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Sanitize String
void sanitize_string(char *str, int max_length, int allow_newlines);

// CP437
char *to_cp437(const char *input);
char *from_cp437(const char *input);

// Starts With
int starts_with(const char *str, const char *prefix);

#ifdef __cplusplus
}
#endif
