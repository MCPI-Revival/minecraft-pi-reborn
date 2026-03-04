#pragma once

#include <string>

// Sanitize String
MCPI_REBORN_UTIL_PUBLIC void sanitize_string(std::string &str, int max_length, bool allow_newlines);

// CP437
MCPI_REBORN_UTIL_PUBLIC unsigned char utf32_to_cp437(char32_t codepoint);
MCPI_REBORN_UTIL_PUBLIC std::string to_cp437(const std::string &input);
MCPI_REBORN_UTIL_PUBLIC std::string from_cp437(const std::string &input);

// Format Time
MCPI_REBORN_UTIL_PUBLIC std::string format_time(const char *fmt);
MCPI_REBORN_UTIL_PUBLIC std::string format_time(const char *fmt, int time);

// Trimming
MCPI_REBORN_UTIL_PUBLIC void trim(std::string &str);

// Convert To Windows String
#ifdef _WIN32
MCPI_REBORN_UTIL_PUBLIC std::wstring convert_utf8_to_wstring(const std::string &str);
MCPI_REBORN_UTIL_PUBLIC std::string convert_wstring_to_utf8(const std::wstring &str);
#endif

// Safe std::to_string
#define def(type) MCPI_REBORN_UTIL_PUBLIC std::string safe_to_string(type x)
def(float);
def(int);
def(size_t);
def(void *);
#undef def
