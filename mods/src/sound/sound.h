#pragma once

#include <string>

__attribute__((visibility("internal"))) std::string _sound_get_source_file();
__attribute__((visibility("internal"))) void _sound_resolve_all();
__attribute__((visibility("internal"))) std::string _sound_pick(std::string sound);
