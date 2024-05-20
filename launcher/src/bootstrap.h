#pragma once

#include <string>

void bootstrap();
void copy_sdk(const std::string &binary_directory, bool log_with_debug);
std::string bootstrap_mods(const std::string &binary_directory);
