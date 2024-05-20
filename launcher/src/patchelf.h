#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MCPI_PATCHED_DIR "/tmp/.minecraft-pi-patched"

void patch_mcpi_elf_dependencies(const char *original_path, char *new_path);

#ifdef __cplusplus
}
#endif
