cmake_minimum_required(VERSION 3.13.0)

project(mods)

add_compile_options(-Wall -Wextra -Werror)
add_link_options(-Wl,--no-undefined)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory(../core core)

include_directories(include)

add_library(core SHARED src/core.c)
target_link_libraries(core dl)

add_library(server SHARED src/server/server.cpp src/server/server_properties.cpp)
target_link_libraries(server core dl SDL)

add_library(extra SHARED src/extra.c src/extra.cpp src/cxx11_util.cpp)
target_link_libraries(extra core dl server)

find_package(glfw3 3.3 REQUIRED)

add_library(compat SHARED src/compat.c)
target_link_libraries(compat core extra SDL GLESv1_CM GLESv2 X11 dl freeimage glfw Xfixes)
# Force GLESv1 Link
target_link_options(compat PRIVATE "-Wl,--no-as-needed")

add_library(readdir SHARED src/readdir.c)
target_link_libraries(readdir core)
