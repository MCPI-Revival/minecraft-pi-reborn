#include <libreborn/elf.h>

// Find And Iterate Over All Segments In Current Binary
typedef struct {
    segment_callback_t callback;
    void *data;
} dl_iterate_callback_data;
static int dl_iterate_callback(struct dl_phdr_info *info, __attribute__((unused)) size_t size, void *data) {
    dl_iterate_callback_data *callback_data = (dl_iterate_callback_data *) data;
    // Only Search Current Program
    if (strcmp(info->dlpi_name, "") == 0) {
        for (int i = 0; i < info->dlpi_phnum; i++) {
            // Only Executable Segemnts
            if (info->dlpi_phdr[i].p_type == PT_LOAD && (info->dlpi_phdr[i].p_flags & PF_X) != 0) {
                // Callback
                (*callback_data->callback)(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr, info->dlpi_phdr[i].p_memsz, callback_data->data);
            }
        }
    }
    return 0;
}
void iterate_segments(segment_callback_t callback, void *data) {
    dl_iterate_callback_data callback_data = {
        .callback = callback,
        .data = data
    };
    dl_iterate_phdr(dl_iterate_callback, (void *) &callback_data);
}
