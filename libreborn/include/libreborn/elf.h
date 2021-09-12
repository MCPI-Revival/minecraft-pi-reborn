#pragma once

#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include <link.h>

#include "log.h"
#include "exec.h"

// Find And Iterate Over All .text Sections In Current Binary
typedef void (*text_section_callback_t)(ElfW(Addr) section, ElfW(Word) size, void *data);
static inline void iterate_text_sections(text_section_callback_t callback, void *data) {
    // Load Main Binary
    FILE *file_obj = fopen("/proc/self/exe", "rb");

    // Verify Binary
    if (!file_obj) {
        ERR("%s", "Unable To Open Current Binary");
    }

    // Get File Size
    fseek(file_obj, 0L, SEEK_END);
    long int file_size = ftell(file_obj);
    fseek(file_obj, 0L, SEEK_SET);

    // Map File To Pointer
    unsigned char *file_map = (unsigned char *) mmap(0, file_size, PROT_READ, MAP_PRIVATE, fileno(file_obj), 0);

    // Parse ELF
    ElfW(Ehdr) *elf_header = (ElfW(Ehdr) *) file_map;
    ElfW(Shdr) *elf_section_headers = (ElfW(Shdr) *) (file_map + elf_header->e_shoff);
    int elf_section_header_count = elf_header->e_shnum;

    // Locate Section Names
    ElfW(Shdr) elf_shstrtab = elf_section_headers[elf_header->e_shstrndx];
    unsigned char *elf_shstrtab_p = file_map + elf_shstrtab.sh_offset;

    // Track .text Sections
    int text_sections = 0;

    // Iterate Sections
    for (int i = 0; i < elf_section_header_count; ++i) {
        ElfW(Shdr) header = elf_section_headers[i];
        char *name = (char *) (elf_shstrtab_p + header.sh_name);
        // Check Section Type
        if (strcmp(name, ".text") == 0) {
            // .text Section
            (*callback)(header.sh_addr, header.sh_size, data);
            text_sections++;
        }
    }

    // Ensure At Least .text Section Was Scanned
    if (text_sections < 1) {
        ERR("%s", "Unable To Find .text Sectons");
    }

    // Unmap And Close File
    munmap(file_map, file_size);
    fclose(file_obj);
}
