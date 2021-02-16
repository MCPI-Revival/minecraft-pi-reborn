cmake_minimum_required(VERSION 3.13.0)

project(mods)

## Setup

add_compile_options(-Wall -Wextra -Werror)
add_link_options(-Wl,--no-undefined)
# Disable C++11 String ABI
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# Add libreborn
add_subdirectory(../libreborn libreborn)

# Include Libraries Exported To Runtime Environment
include_directories(/app/export/include)

# Find GLFW
find_package(glfw3 3.3 REQUIRED)

## Mods

add_library(compat SHARED src/compat/compat.c)
target_link_libraries(compat feature input screenshot SDL GLESv1_CM X11 dl glfw Xfixes)

add_library(readdir SHARED src/readdir/readdir.c)

add_library(feature SHARED src/feature/feature.c)
target_link_libraries(feature reborn)

add_library(server SHARED src/server/server.cpp src/server/server_properties.cpp)
target_link_libraries(server reborn feature dl SDL pthread)

add_library(screenshot SHARED src/screenshot/screenshot.c)
target_link_libraries(screenshot reborn GLESv1_CM freeimage)

add_library(camera SHARED src/camera/camera.cpp)
target_link_libraries(camera reborn screenshot)

add_library(game_mode SHARED src/game_mode/game_mode.c src/game_mode/game_mode.cpp)
target_link_libraries(game_mode reborn)

add_library(input SHARED src/input/input.c src/input/input.cpp)
target_link_libraries(input reborn feature SDL chat)

add_library(misc SHARED src/misc/misc.c src/misc/misc.cpp)
target_link_libraries(misc reborn feature util)

add_library(options SHARED src/options/options.c)
target_link_libraries(options reborn feature)

add_library(override SHARED src/override/override.c)
target_link_libraries(override reborn dl)

add_library(textures SHARED src/textures/textures.cpp)
target_link_libraries(textures reborn feature GLESv1_CM)

add_library(chat SHARED src/chat/chat.cpp src/chat/ui.c)
target_link_libraries(chat reborn pthread)

add_library(test SHARED src/test/test.c)
target_link_libraries(test reborn)

add_library(init SHARED src/init/init.c)
target_link_libraries(init compat server game_mode camera input misc options textures chat test)

## Stubs

# Stub RPI-Specific Graphics
add_library(bcm_host SHARED src/stubs/bcm_host.c)

# Stub EGL
add_library(EGL SHARED src/stubs/EGL.c)
target_link_libraries(EGL compat)

# MCPI Links Against GLESv2 But Uses GLESv1_CM
add_library(GLESv2 SHARED src/stubs/GLESv2.c)
target_link_libraries(GLESv2 GLESv1_CM)
target_link_options(GLESv2 PRIVATE "-Wl,--no-as-needed")

## Install
install(TARGETS init compat readdir feature screenshot override server game_mode camera input misc options textures chat test bcm_host EGL GLESv2 DESTINATION /mods)