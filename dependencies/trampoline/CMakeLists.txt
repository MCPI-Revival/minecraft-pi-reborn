project(trampoline)

## Trampoline Host

# QEMU
set(QEMU_VERSION "9.0.0")
set(TRAMPOLINE_QEMU_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../archives/qemu-${QEMU_VERSION}.tar.xz")
if(NOT MCPI_USE_TRAMPOLINE_HOST)
    set(TRAMPOLINE_HEADERS_ONLY TRUE)
endif()

# Build
add_subdirectory(src)

# Install
install_trampoline("${MCPI_BIN_DIR}" "${MCPI_LEGAL_DIR}")