#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void patch_mcpi_elf_dependencies(const char *original_path, const char *linker);
char *patch_get_interpreter(const char *file);

#ifdef __cplusplus
}
#endif
