#pragma once

#include <string>
#include <functional>

// Paths
void chop_last_component(std::string &str);
std::string safe_realpath(const std::string &path);
bool read_directory(const std::string &path, const std::function<void(const struct dirent *)> &callback, bool allow_nonexistent_dir = false);
void make_directory(std::string path);
void copy_file(const std::string &src, const std::string &dst, bool log = false);

// Binary
std::string get_binary();
std::string get_binary_directory();
std::string get_appimage_path();

// SDK
void copy_sdk(const std::string &binary_directory, bool force);

// Copying Desktop File
bool is_desktop_file_installed();
void copy_desktop_file();

// Environment
void setup_home();