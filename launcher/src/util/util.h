#pragma once

#include <string>
#include <functional>

// Paths
MCPI_LAUNCHER_UTIL_PUBLIC void chop_last_component(std::string &str);
MCPI_LAUNCHER_UTIL_PUBLIC std::string safe_realpath(const std::string &path);
MCPI_LAUNCHER_UTIL_PUBLIC bool read_directory(const std::string &path, const std::function<void(const struct dirent *, bool)> &callback, bool allow_nonexistent_dir = false);
MCPI_LAUNCHER_UTIL_PUBLIC void make_directory(std::string path);
MCPI_LAUNCHER_UTIL_PUBLIC void delete_recursively(const std::string &path, bool allow_nonexistent_dir);
MCPI_LAUNCHER_UTIL_PUBLIC void copy_file(const std::string &src, const std::string &dst, bool log = false);

// Binary
MCPI_LAUNCHER_UTIL_PUBLIC std::string get_binary();
MCPI_LAUNCHER_UTIL_PUBLIC std::string get_binary_directory();
MCPI_LAUNCHER_UTIL_PUBLIC std::string get_appimage_path();

// SDK
MCPI_LAUNCHER_UTIL_PUBLIC void copy_sdk(const std::string &binary_directory, bool force);

// Environment
MCPI_LAUNCHER_UTIL_PUBLIC void setup_home();