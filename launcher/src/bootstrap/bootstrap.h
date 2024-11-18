#pragma once

#include <string>
#include <vector>

#include "../options/parser.h"

#define MCPI_PATCHED_DIR "/tmp/.minecraft-pi-patched"

void bootstrap(const options_t &options);
// Debugging
void print_debug_information();
// Mods
std::vector<std::string> bootstrap_mods(const std::string &binary_directory);
// Assets
void bootstrap_assets(const std::string &original_game_binary);
// ELF
std::string get_new_linker(const std::string &binary_directory);
std::vector<std::string> get_ld_path(const std::string &binary_directory);
void patch_mcpi_elf_dependencies(const std::string &original_path, char *new_path, const std::string &interpreter, const std::vector<std::string> &rpath, const std::vector<std::string> &mods);