project(libreborn)

# Config
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/libreborn")
configure_file(include/libreborn/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/libreborn/config.h" ESCAPE_QUOTES @ONLY)

# Util
add_library(reborn-util SHARED
    src/util/exec.cpp
    src/util/string.cpp
    src/util/util.cpp
    src/util/log.cpp
    src/util/cp437.cpp
    src/util/env.cpp
    src/util/config.cpp
    src/util/flags/node.cpp
    src/util/flags/flags.cpp
    src/util/flags/available-feature-flags # Show In IDE
)
embed_resource(reborn-util src/util/flags/available-feature-flags)
target_link_libraries(reborn-util PRIVATE utf8cpp)
if(TARGET glfw)
    target_sources(reborn-util PRIVATE src/util/glfw.cpp)
    target_link_libraries(reborn-util PRIVATE glfw)
endif()
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
