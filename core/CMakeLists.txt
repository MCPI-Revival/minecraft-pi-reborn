cmake_minimum_required(VERSION 3.1.0)

project(core)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

add_library(core SHARED src/core.c)
target_link_libraries(core dl)

add_library(bcm_host SHARED src/bcm_host.c)

add_executable(launcher src/launcher.c)
