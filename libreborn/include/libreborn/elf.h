#pragma once

#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include <link.h>

#include "log.h"
#include "exec.h"

#ifdef __cplusplus
extern "C" {
#endif

// Find And Iterate Over All .text Sections In Current Binary
typedef void (*text_section_callback_t)(ElfW(Addr) section, ElfW(Word) size, void *data);
void iterate_text_sections(const char *exe, text_section_callback_t callback, void *data);

#ifdef __cplusplus
}
#endif
