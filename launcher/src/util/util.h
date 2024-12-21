#pragma once

#include <string>
#include <functional>

void chop_last_component(std::string &str);
std::string safe_realpath(const std::string &path);
std::string get_binary_directory();

void copy_sdk(const std::string &binary_directory, bool force);

void setup_path();
void setup_home();

bool read_directory(const std::string &path, const std::function<void(const struct dirent *)> &callback, bool allow_nonexistent_dir = false);