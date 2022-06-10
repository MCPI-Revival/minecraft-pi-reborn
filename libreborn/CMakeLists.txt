project(libreborn)

# Config
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/libreborn")
configure_file(include/libreborn/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/libreborn/config.h" ESCAPE_QUOTES @ONLY)

# Util
add_library(reborn-util STATIC src/util/elf.c src/util/exec.c src/util/string.c src/util/util.c)
target_include_directories(reborn-util PUBLIC include PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")

# Patch
if(BUILD_ARM_COMPONENTS)
    add_library(reborn-patch SHARED src/patch/patch.c)
    target_link_libraries(reborn-patch dl pthread reborn-util)
    target_compile_definitions(reborn-patch PUBLIC -DREBORN_HAS_PATCH_CODE)
    # Install
    install(TARGETS reborn-patch DESTINATION "${MCPI_LIB_DIR}")
endif()
