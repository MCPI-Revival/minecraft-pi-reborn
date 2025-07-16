#include <vector>
#include <cstring>

#include <elf.h>
#include <dlfcn.h>
#include <link.h>

#include <libreborn/patch.h>
#include "patch-internal.h"

// Track Segments
static std::vector<segment_data> &get_segments() {
    static std::vector<segment_data> data;
    return data;
}

// Functions
segment_data &get_data_for_addr(void *addr) {
    for (segment_data &data : get_segments()) {
        if (addr >= data.start && addr < data.end) {
            return data;
        }
    }
    ERR("Address Not Part Of Main Program: %p", addr);
}
void add_segment(segment_data data) {
    get_segments().push_back(data);
}

// Init
void reborn_init_patch() {
    dl_iterate_phdr([](dl_phdr_info *info, MCPI_UNUSED size_t size, MCPI_UNUSED void *user_data) {
        // Only Search Current Program
        if (strcmp(info->dlpi_name, "") == 0) {
            for (int i = 0; i < info->dlpi_phnum; i++) {
                // Only Loaded Segments
                if (info->dlpi_phdr[i].p_type == PT_LOAD) {
                    // Store
                    segment_data data = {};
                    data.start = (void *) (info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
                    data.end = (void *) (((uintptr_t) data.start) + info->dlpi_phdr[i].p_memsz);
                    data.is_executable = info->dlpi_phdr[i].p_flags & PF_X;
                    data.is_writable = info->dlpi_phdr[i].p_flags & PF_W;
                    add_segment(data);
                }
            }
        }
        // Return
        return 0;
    }, nullptr);
    // Init Callsite Cache
    init_cache();
}
