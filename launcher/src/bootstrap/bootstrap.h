#pragma once

#include <string>
#include <vector>

void bootstrap();
// Debugging
void print_debug_information();
// Mods
std::vector<std::string> bootstrap_mods(const std::string &binary_directory);
// Assets
void bootstrap_assets(const std::string &original_game_binary);
// ELF
extern const char *patched_exe_path;
std::string get_new_linker(const std::string &binary_directory);
std::vector<std::string> get_ld_path(const std::string &binary_directory);
void patch_mcpi_elf_dependencies(const std::string &original_path, const std::string &interpreter, const std::vector<std::string> &rpath, const std::vector<std::string> &mods);