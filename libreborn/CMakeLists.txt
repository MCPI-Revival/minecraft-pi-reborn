project(libreborn)

# Config
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/libreborn")
configure_file(include/libreborn/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/libreborn/config.h" ESCAPE_QUOTES @ONLY)

# Util
add_library(reborn-util SHARED
    src/util/exec.c
    src/util/string.c
    src/util/util.c
    src/util/log.c
    src/util/cp437.cpp
    src/util/env.c
)
target_include_directories(
    reborn-util
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    "$<INSTALL_INTERFACE:${MCPI_SDK_INCLUDE_DIR}/libreborn>"
)
target_link_libraries(reborn-util PRIVATE utf8cpp)
# Install
install(TARGETS reborn-util DESTINATION "${MCPI_LIB_DIR}")
# SDK
if(BUILD_ARM_COMPONENTS)
    install(TARGETS reborn-util EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
    install(DIRECTORY "include/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/libreborn")
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/" DESTINATION "${MCPI_SDK_INCLUDE_DIR}/libreborn")
endif()

# Patch
if(BUILD_ARM_COMPONENTS)
    add_library(reborn-patch SHARED
        src/patch/patch.cpp
        src/patch/segments.cpp
        src/patch/code-block.cpp
        src/patch/instruction.cpp
    )
    target_link_libraries(reborn-patch dl pthread reborn-util)
    target_compile_definitions(reborn-patch PUBLIC -DREBORN_HAS_PATCH_CODE)
    # Install
    install(TARGETS reborn-patch DESTINATION "${MCPI_LIB_DIR}")
    # SDK
    install(TARGETS reborn-patch EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
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
    install(TARGETS fake-libpng DESTINATION "${MCPI_LIB_DIR}")
endif()
