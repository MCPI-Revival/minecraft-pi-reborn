project(libreborn)

add_library(reborn-headers INTERFACE)
target_include_directories(reborn-headers INTERFACE include)

if(BUILD_ARM_COMPONENTS)
    add_library(reborn SHARED src/libreborn.c)
    target_link_libraries(reborn dl reborn-headers)
    # Install
    install(TARGETS reborn DESTINATION "${MCPI_LIB_DIR}")
endif()
