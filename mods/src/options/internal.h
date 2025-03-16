#pragma once

#include <libreborn/config.h>
#include <symbols/minecraft.h>

#define SOUND_DOC_URL MCPI_DOCS_GETTING_STARTED "#sound"

__attribute__((visibility("internal"))) void _init_options_ui();
__attribute__((visibility("internal"))) extern Options *stored_options;
__attribute__((visibility("internal"))) Screen *_create_options_info_screen();