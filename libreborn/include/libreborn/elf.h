#pragma once

#ifdef __arm__
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>

#include "log.h"

// Find And Iterate Over All .text Sections In Current Binary
typedef void (*text_section_callback_t)(void *section, Elf32_Word size, void *data);
static inline void iterate_text_sections(text_section_callback_t callback, void *data) {
    // Load Main Binary
    char *real_path = realpath("/proc/self/exe", NULL);
    FILE *file_obj = fopen(real_path, "rb");

    // Verify Binary
    if (!file_obj) {
        ERR("Unable To Open Binary: %s", real_path);
    }

    // Get File Size
    fseek(file_obj, 0L, SEEK_END);
    long int size = ftell(file_obj);
    fseek(file_obj, 0L, SEEK_SET);

    // Map File To Pointer
    unsigned char *file_map = (unsigned char *) mmap(0, size, PROT_READ, MAP_PRIVATE, fileno(file_obj), 0);

    // Parse ELF
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *) file_map;
    Elf32_Shdr *elf_section_headers = (Elf32_Shdr *) (file_map + elf_header->e_shoff);
    int elf_section_header_count = elf_header->e_shnum;

    // Locate Section Names
    Elf32_Shdr elf_strtab = elf_section_headers[elf_header->e_shstrndx];
    unsigned char *elf_strtab_p = file_map + elf_strtab.sh_offset;

    // Track .text Sections
    int text_sections = 0;

    // Iterate Sections
    for (int i = 0; i < elf_section_header_count; ++i) {
        Elf32_Shdr header = elf_section_headers[i];
        char *name = (char *) (elf_strtab_p + header.sh_name);
        // Check Section Type
        if (strcmp(name, ".text") == 0) {
            // .text Section
            (*callback)((void *) header.sh_addr, header.sh_size, data);
            text_sections++;
        }
    }

    // Ensure At Least .text Section Was Scanned
    if (text_sections < 1) {
        ERR("Unable To Find .text Sectons On: %s", real_path);
    }

    // Free Binary Path
    free(real_path);

    // Unmap And Close File
    munmap(file_map, size);
    fclose(file_obj);
}
#endif // #ifdef __arm__
