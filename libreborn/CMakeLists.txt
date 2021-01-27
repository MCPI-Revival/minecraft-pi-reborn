cmake_minimum_required(VERSION 3.13.0)

project(libreborn)

add_compile_options(-Wall -Wextra -Werror)
add_link_options(-Wl,--no-undefined)

add_library(reborn SHARED src/libreborn.c)
target_link_libraries(reborn dl)
target_include_directories(reborn PUBLIC include)

# Install
install(TARGETS reborn DESTINATION /mods)