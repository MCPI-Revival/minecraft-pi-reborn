#pragma once

#include <symbols/minecraft.h>

__attribute__((visibility("internal"))) extern const std::string sound_doc_url;

__attribute__((visibility("internal"))) void _init_options_ui();
__attribute__((visibility("internal"))) extern Options *stored_options;
__attribute__((visibility("internal"))) Screen *_create_options_info_screen();