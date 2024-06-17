#pragma once

#include <string>

void run_simple_command(const char *const command[], const char *error);

void chop_last_component(std::string &str);
std::string safe_realpath(const std::string &path);
std::string get_binary_directory();
