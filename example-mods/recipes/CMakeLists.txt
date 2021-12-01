cmake_minimum_required(VERSION 3.16.0)

# Build For ARM
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Start Project
project(recipes)

# Include SDK
include("$ENV{HOME}/.minecraft-pi/sdk/lib/minecraft-pi-reborn-legacy-client/sdk/sdk.cmake")

# Build
add_library(recipes SHARED recipes.cpp)
target_link_libraries(recipes mods-headers reborn-util symbols misc)
