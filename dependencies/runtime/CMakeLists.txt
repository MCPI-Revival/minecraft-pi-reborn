project(runtime)

## Extra Runtime

# QEMU
set(QEMU_VERSION "9.1.1")
set(RUNTIME_QEMU_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../archives/qemu-${QEMU_VERSION}.tar.xz" CACHE FILEPATH "" FORCE)
if(NOT BUILD_NATIVE_COMPONENTS)
    set(TRAMPOLINE_HEADERS_ONLY TRUE CACHE BOOL "" FORCE)
endif()

# Build
add_subdirectory(src)