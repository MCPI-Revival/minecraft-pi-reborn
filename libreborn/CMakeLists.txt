project(libreborn)

# Config
set(CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/config.cpp")
configure_file(src/util/config.cpp.in "${CONFIG_FILE}" ESCAPE_QUOTES @ONLY)

# Util
add_library(reborn-util SHARED
    src/util/exec/exec.cpp
    src/util/exec/poll.cpp
    src/util/exec/stdio.cpp
    src/util/string.cpp
    src/util/util.cpp
    src/util/log.cpp
    src/util/cp437.cpp
    src/util/env/env.cpp
    "${CONFIG_FILE}"
    src/util/env/flags/node.cpp
    src/util/env/flags/flags.cpp
    src/util/env/flags/available-feature-flags # Show In IDE
    src/util/env/servers.cpp
)
embed_resource(reborn-util src/util/env/flags/available-feature-flags)
target_link_libraries(reborn-util PRIVATE utf8cpp)
if(TARGET glfw)
    target_sources(reborn-util PRIVATE
        src/util/glfw/glfw.cpp
    )
    target_link_libraries(reborn-util PRIVATE
        glfw
    )
    if(MCPI_WIN32)
        target_sources(reborn-util PRIVATE
            src/util/glfw/icon_win32.cpp
        )
    endif()
endif()
target_compile_definitions(reborn-util PUBLIC
    "MCPI_UNUSED=__attribute__((unused))"
    "MCPI_INTERNAL=__attribute__((visibility(\"internal\")))"
)
setup_header_dirs(reborn-util
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
)
setup_library(reborn-util TRUE TRUE)

# Patch
if(BUILD_ARM_COMPONENTS)
    add_library(reborn-patch SHARED
        src/patch/patch.cpp
        src/patch/segments.cpp
        src/patch/code-block.cpp
        src/patch/instruction.cpp
        src/patch/cache.cpp
    )
    target_link_libraries(reborn-patch dl pthread reborn-util)
    target_compile_definitions(reborn-patch PUBLIC REBORN_HAS_PATCH_CODE)
    # Install
    setup_library(reborn-patch TRUE TRUE)
endif()

# Fake LibPNG To Satisfy Symbol Versioning Requirement
if(BUILD_ARM_COMPONENTS)
    add_library(fake-libpng SHARED src/fake-libpng/empty.c)
    set_target_properties(fake-libpng PROPERTIES
        OUTPUT_NAME "png12"
        SOVERSION 0
    )
    target_link_options(fake-libpng PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/fake-libpng/empty.vers")
    # Install
    setup_library(fake-libpng TRUE FALSE)
endif()
