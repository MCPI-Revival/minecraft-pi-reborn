#pragma once

#include <string>
#include <vector>

#include "../options/parser.h"

// Bootstrap
void bootstrap(const options_t &options);

// Utility Functions
std::string run_command_and_trim(const char *const command[], const char *action);
std::string translate_native_path_to_linux(const std::string &path);
std::string get_temp_dir();

// Debugging
void print_debug_information();
// Mods
std::vector<std::string> bootstrap_mods(const std::string &binary_directory);
// Assets
void bootstrap_assets(const std::string &original_game_binary);
// ELF
std::string get_patched_exe_path();
std::string get_new_linker(const std::string &binary_directory_linux);
std::vector<std::string> get_ld_path(const std::string &binary_directory_linux);
void patch_mcpi_elf_dependencies(const std::string &original_path, const std::string &interpreter, const std::vector<std::string> &rpath, const std::vector<std::string> &mods);