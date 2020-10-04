cmake_minimum_required(VERSION 3.1.0)

project(mods)

add_compile_options(-Wall -Wextra -Werror)

add_subdirectory(../core core)

include_directories(include)

add_library(core SHARED src/core.c)
target_link_libraries(core dl)

add_library(extra SHARED src/extra.c src/extra.cpp)
target_link_libraries(extra core dl)

add_library(compat SHARED src/compat.c)
target_link_libraries(compat core extra SDL EGL GLESv1_CM GLESv2 X11 dl freeimage)
# Force GLESv1 Link
target_link_options(compat PRIVATE "-Wl,--no-as-needed")

add_library(readdir SHARED src/readdir.c)
target_link_libraries(readdir core)
