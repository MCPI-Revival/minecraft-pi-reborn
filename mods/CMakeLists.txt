cmake_minimum_required(VERSION 3.13.0)

project(mods)

add_compile_options(-Wall -Wextra -Werror)
add_link_options(-Wl,--no-undefined)
# Disable C++11 String ABI
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory(../core core)

# Include Libraries Exported To Runtime Environment
include_directories(/app/export/include)

# Include Headers
include_directories(include)

# Find GLFW
find_package(glfw3 3.3 REQUIRED)

add_library(compat SHARED src/compat/compat.c)
target_link_libraries(compat feature input screenshot SDL GLESv1_CM X11 dl glfw Xfixes)

add_library(readdir SHARED src/compat/readdir.c)

add_library(core SHARED src/core/core.c)
target_link_libraries(core dl)

add_library(feature SHARED src/feature/feature.c)

add_library(server SHARED src/server/server.cpp src/server/server_properties.cpp src/server/playerdata.cpp)
target_link_libraries(server core feature dl SDL pthread)

add_library(screenshot SHARED src/screenshot/screenshot.c)
target_link_libraries(screenshot GLESv1_CM freeimage)

add_library(camera SHARED src/camera/camera.cpp)
target_link_libraries(camera core screenshot)

add_library(game_mode SHARED src/game_mode/game_mode.c src/game_mode/game_mode.cpp)
target_link_libraries(game_mode core)

add_library(input SHARED src/input/input.c src/input/input.cpp)
target_link_libraries(input core feature SDL)

add_library(misc SHARED src/misc/misc.c src/misc/misc.cpp)
target_link_libraries(misc core feature util)

add_library(options SHARED src/options/options.c)
target_link_libraries(options core feature)

add_library(override SHARED src/override/override.c)
target_link_libraries(override dl)

add_library(textures SHARED src/textures/textures.cpp)
target_link_libraries(textures core feature GLESv1_CM)

add_library(init SHARED src/init/init.c)
target_link_libraries(init compat server game_mode camera input misc options textures)
