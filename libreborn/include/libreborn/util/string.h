#pragma once

#include <string>

// Sanitize String
void sanitize_string(std::string &str, int max_length, bool allow_newlines);

// CP437
std::string to_cp437(const std::string &input);
std::string from_cp437(const std::string &input);

// Format Time
std::string format_time(const char *fmt);
std::string format_time(const char *fmt, int time);