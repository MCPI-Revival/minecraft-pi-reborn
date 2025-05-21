#pragma once

#include <symbols/minecraft.h>

MCPI_INTERNAL extern const std::string sound_doc_url;

MCPI_INTERNAL void _init_options_ui();
MCPI_INTERNAL extern Options *stored_options;
MCPI_INTERNAL Screen *_create_options_info_screen();