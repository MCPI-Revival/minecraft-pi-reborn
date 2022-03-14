project(libreborn)

add_library(reborn-util STATIC src/util/elf.c src/util/exec.c src/util/string.c src/util/util.c)
target_include_directories(reborn-util PUBLIC include)

if(BUILD_ARM_COMPONENTS)
    add_library(reborn-patch SHARED src/patch/patch.c)
    target_link_libraries(reborn-patch dl reborn-util)
    target_compile_definitions(reborn-patch PUBLIC -DREBORN_HAS_PATCH_CODE)
    # Install
    install(TARGETS reborn-patch DESTINATION "${MCPI_LIB_DIR}")
endif()
