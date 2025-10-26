#pragma once

#include <string>

// Sanitize String
void sanitize_string(std::string &str, int max_length, bool allow_newlines);

// CP437
unsigned char utf32_to_cp437(char32_t codepoint);
std::string to_cp437(const std::string &input);
std::string from_cp437(const std::string &input);

// Format Time
std::string format_time(const char *fmt);
std::string format_time(const char *fmt, int time);

// Trimming
void trim(std::string &str);

// Convert To Windows String
#ifdef _WIN32
std::wstring convert_utf8_to_wstring(const std::string &str);
#endif

// Safe std::to_string
#define def(type) std::string safe_to_string(type x)
def(float);
def(int);
def(size_t);
#undef def
