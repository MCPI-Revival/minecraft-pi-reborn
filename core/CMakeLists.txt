cmake_minimum_required(VERSION 3.1.0)

project(core)

add_compile_options(-Wall -Wextra -Werror)

add_library(bcm_host SHARED src/stubs/bcm_host.c)
add_library(EGL SHARED src/stubs/EGL.c)

add_executable(launcher src/launcher.c)
