#pragma once

#define replace_func(name) \
    overwrite_call((void *) (name)->backup, name, name##_injection, true)

void _init_display_lists_chunks_render();
void _init_display_lists_font();
void _init_display_lists_tesselator();

unsigned int _display_lists_get_for_buffer(unsigned int buffer);