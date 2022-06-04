#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MCPI_PATCHED_DIR "/tmp/.minecraft-pi-patched"

void patch_mcpi_elf_dependencies(const char *original_path, char *new_path, const char *linker);
char *patch_get_interpreter(const char *file);

#ifdef __cplusplus
}
#endif
