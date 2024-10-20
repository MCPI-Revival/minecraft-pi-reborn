#pragma once

#include <string>
#include <vector>

#define MCPI_PATCHED_DIR "/tmp/.minecraft-pi-patched"

void patch_mcpi_elf_dependencies(const std::string &original_path, char *new_path, const std::string &interpreter, const std::vector<std::string> &rpath, const std::vector<std::string> &mods);
