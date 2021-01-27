#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <elf.h>
#include <errno.h>

#include <libreborn/libreborn.h>

// Find And Iterate Over All .text Sections In Current Binary
typedef void (*text_section_callback)(void *section, Elf32_Word size, void *data);
static void iterate_text_section(text_section_callback callback, void *data) {
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
    unsigned char *file_map = mmap(0, size, PROT_READ, MAP_PRIVATE, fileno(file_obj), 0);

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
        if (strcmp(name, ".text") == 0) {
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

// BL Instruction Magic Number
#define BL_INSTRUCTION 0xeb

// Generate A BL Instruction
static uint32_t generate_bl_instruction(void *from, void *to) {
    uint32_t instruction;
    unsigned char *instruction_array = (unsigned char *) &instruction;

    instruction_array[3] = BL_INSTRUCTION;

    unsigned char *pc = ((unsigned char *) from) + 8;
    int32_t offset = (int32_t) to - (int32_t) pc;
    int32_t target = offset >> 2;

    unsigned char *target_array = (unsigned char *) &target;
    instruction_array[0] = target_array[0];
    instruction_array[1] = target_array[1];
    instruction_array[2] = target_array[2];

    return instruction;
}

// Run For Every .text Section
struct overwrite_data {
    const char *file;
    int line;
    void *target;
    void *replacement;
    int found;
};
static void overwrite_calls_callback(void *section, Elf32_Word size, void *data) {
    struct overwrite_data *args = (struct overwrite_data *) data;

    for (uint32_t i = 0; i < size; i = i + 4) {
        unsigned char *addr = ((unsigned char *) section) + i;
        if (addr[3] == BL_INSTRUCTION) {
            uint32_t check_instruction = generate_bl_instruction(addr, args->target);
            unsigned char *check_instruction_array = (unsigned char *) &check_instruction;
            if (addr[0] == check_instruction_array[0] && addr[1] == check_instruction_array[1] && addr[2] == check_instruction_array[2]) {
                uint32_t new_instruction = generate_bl_instruction(addr, args->replacement);
                _patch(args->file, args->line, addr, (unsigned char *) &new_instruction);
                args->found++;
            }
        }
    }
}

// Limit To 512 overwrite_calls() Uses
#define CODE_BLOCK_SIZE 4096
static unsigned char *code_block = NULL;
#define CODE_SIZE 8
static int code_block_remaining = CODE_BLOCK_SIZE;

static void update_code_block(void *target) {
    // BL Instructions Can Only Access A Limited Portion of Memory, So This Allocates Memory Closer To The Original Instruction, That When Run, Will Jump Into The Actual Target
    if (code_block == NULL) {
        code_block = mmap((void *) 0x200000, CODE_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (code_block == MAP_FAILED) {
            ERR("Unable To Allocate Code Block: %s", strerror(errno));
        }
        INFO("Code Block Allocated At: 0x%08x", (uint32_t) code_block);
    }
    if (code_block_remaining < CODE_SIZE) {
        ERR("%s", "Maximum Amount Of overwrite_calls() Uses Reached");
    }
    _overwrite(NULL, -1, code_block, target);
}
static void increment_code_block() {
    code_block = code_block + CODE_SIZE;
    code_block_remaining = code_block_remaining - CODE_SIZE;
}

// Overwrite Specific BL Instruction
void _overwrite_call(const char *file, int line, void *start, void *target) {
    // Add New Target To Code Block
    update_code_block(target);

    uint32_t new_instruction = generate_bl_instruction(start, code_block);
    _patch(file, line, start, (unsigned char *) &new_instruction);

    // Increment Code Block Position
    increment_code_block();
}

// Overwrite Function Calls
void _overwrite_calls(const char *file, int line, void *start, void *target) {
    // Add New Target To Code Block
    update_code_block(target);

    struct overwrite_data data;
    data.file = file;
    data.line = line;
    data.target = start;
    data.replacement = code_block;
    data.found = 0;

    iterate_text_section(overwrite_calls_callback, &data);

    // Increment Code Block Position
    increment_code_block();

    if (data.found < 1) {
        ERR("(%s:%i) Unable To Find Callsites For 0x%08x", file, line, (uint32_t) start);
    }
}

// Overwrite Function
void _overwrite(const char *file, int line, void *start, void *target) {
    unsigned char patch_data[4] = {0x04, 0xf0, 0x1f, 0xe5};

    _patch(file, line, start, patch_data);
    _patch_address(file, line, start + 4, target);
}

// Print Patch Debug Data
#define PATCH_PRINTF(file, line, start, str) if (file != NULL) fprintf(stderr, "[PATCH]: (%s:%i) Patching (0x%08x) - "str": 0x%02x 0x%02x 0x%02x 0x%02x\n", file, line, (uint32_t) start, data[0], data[1], data[2], data[3]);

// Patch Instruction
void _patch(const char *file, int line, void *start, unsigned char patch[]) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t end = ((uintptr_t) start) + 4;
    uintptr_t page_start = ((uintptr_t) start) & -page_size;
    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_WRITE);

    unsigned char *data = (unsigned char *) start;

    PATCH_PRINTF(file, line, start, "original");

    memcpy(data, patch, 4);

    PATCH_PRINTF(file, line, start, "result");

    mprotect((void *) page_start, end - page_start, PROT_READ | PROT_EXEC);

    // Clear ARM Instruction Cache
    __clear_cache(start, (void *) end);
}

// Patch Address
void _patch_address(const char *file, int line, void *start, void *target) {
    uint32_t addr = (uint32_t) target;
    unsigned char patch_data[4] = {addr & 0xff, (addr >> 8) & 0xff, (addr >> 16) & 0xff, (addr >> 24) & 0xff};
    _patch(file, line, start, patch_data);
}
