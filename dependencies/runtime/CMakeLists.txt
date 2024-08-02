project(runtime)

## Extra Runtime

# QEMU
set(QEMU_VERSION "9.0.2")
set(RUNTIME_QEMU_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../archives/qemu-${QEMU_VERSION}.tar.xz")
if(NOT BUILD_NATIVE_COMPONENTS)
    set(TRAMPOLINE_HEADERS_ONLY TRUE)
endif()

# Build
add_subdirectory(src)

# Install
if(COMMAND install_runtime)
    install_runtime("${MCPI_BIN_DIR}" "${MCPI_LEGAL_DIR}")
endif()