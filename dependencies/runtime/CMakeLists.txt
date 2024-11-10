project(runtime)

## Extra Runtime

# QEMU
set(QEMU_VERSION "9.1.1")
file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../archives/qemu-${QEMU_VERSION}.tar.xz" QEMU_PATH)
set(RUNTIME_QEMU_ARCHIVE "${QEMU_PATH}" CACHE FILEPATH "" FORCE)
if(NOT BUILD_NATIVE_COMPONENTS)
    set(TRAMPOLINE_HEADERS_ONLY TRUE CACHE BOOL "" FORCE)
endif()

# Build
add_subdirectory(src)