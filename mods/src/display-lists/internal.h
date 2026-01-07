#pragma once

#define replace_func(name) \
    overwrite_call((void *) (name)->backup, name, name##_injection, true)

MCPI_INTERNAL void _init_display_lists_chunks_render();
MCPI_INTERNAL void _init_display_lists_font();
MCPI_INTERNAL void _init_display_lists_tesselator();

MCPI_INTERNAL unsigned int _display_lists_get_for_buffer(unsigned int buffer);