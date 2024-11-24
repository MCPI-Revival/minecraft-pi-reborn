project(runtime)

## Extra Runtime

# QEMU
set(QEMU_VERSION "9.1.1")
force_set(RUNTIME_QEMU_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../archives/qemu-${QEMU_VERSION}.tar.xz" FILEPATH)
if(NOT BUILD_NATIVE_COMPONENTS)
    force_set(TRAMPOLINE_HEADERS_ONLY TRUE BOOL)
endif()

# Build
add_subdirectory(src)

# RPath
if(TARGET runtime)
    set_target_properties(runtime PROPERTIES INSTALL_RPATH "$ORIGIN/../lib/native")
    target_link_options(runtime PRIVATE "LINKER:--disable-new-dtags")
endif()