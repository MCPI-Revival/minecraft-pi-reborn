cmake_minimum_required(VERSION 3.1.0)

project(core)

add_compile_options(-Wall -Wextra -Werror)

add_library(bcm_host SHARED src/stubs/bcm_host.c)
add_library(EGL SHARED src/stubs/EGL.c)

# MCPI Links Against GLESv2 But Requires GLESv1_CM
add_library(GLESv2 SHARED src/stubs/GLESv2.c)
target_link_libraries(GLESv2 GLESv1_CM)
target_link_options(GLESv2 PRIVATE "-Wl,--no-as-needed")

add_executable(launcher src/launcher.c)
