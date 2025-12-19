#pragma once

#define replace(name) \
    overwrite_call((void *) (name)->backup, name, name##_injection, true)

MCPI_INTERNAL void _init_display_lists_chunks_rebuild();
MCPI_INTERNAL void _init_display_lists_chunks_render();
MCPI_INTERNAL void _init_display_lists_font();
MCPI_INTERNAL void _init_display_lists_tesselator();