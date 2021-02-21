cmake_minimum_required(VERSION 3.1.0)

project(launcher)

add_compile_options(-Wall -Wextra -Werror)

## Launcher

add_executable(launcher src/launcher.c)

# Install
install(TARGETS launcher DESTINATION /)
install(PROGRAMS src/run.sh DESTINATION /)

## Stubs

# Stub RPI-Specific Graphics
add_library(bcm_host SHARED src/stubs/bcm_host.c)

# Stub EGL
add_library(EGL SHARED src/stubs/EGL.c)

# Stub SDL
add_library(SDL-1.2 SHARED src/stubs/SDL.cpp)
target_link_libraries(SDL-1.2 pthread)
set_target_properties(SDL-1.2 PROPERTIES SOVERSION "0")

# MCPI Links Against GLESv2 But Uses GLESv1_CM
add_library(GLESv2 SHARED src/stubs/GLESv2.c)
target_link_libraries(GLESv2 GLESv1_CM)
target_link_options(GLESv2 PRIVATE "-Wl,--no-as-needed")

# Install Stubs
install(TARGETS bcm_host EGL SDL-1.2 GLESv2 DESTINATION /lib)