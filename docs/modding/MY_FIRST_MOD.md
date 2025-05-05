# My First Mod
This chapter explains how to set up the SDK and build a simple mod.

## Installing The SDK
1. Install [CMake](https://cmake.org/). This is `cmake` and `ninja-build` on Ubuntu.
2. Install an ARM32 C++ toolchain. This is `g++-arm-linux-gnueabihf` on Ubuntu.
3. Install MCPI-Reborn.
4. Run MCPI-Reborn. This will copy the SDK to the profile directory.

## Creating A Mod
1. Create a folder for your new mod's source code. All following steps should take place in this directory.
2. Create `CMakeLists.txt`:
   ```cmake
   cmake_minimum_required(VERSION 3.16.0)

   # Build For ARM
   set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
   set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
   set(CMAKE_SYSTEM_NAME "Linux")
   set(CMAKE_SYSTEM_PROCESSOR "arm")

   # Start Project
   project(<your-mod-name>)

   # Include SDK
   include("$ENV{HOME}/.minecraft-pi/sdk/lib/minecraft-pi-reborn/sdk/sdk.cmake")

   # Build
   add_library(<your-mod-name> SHARED src/main.cpp)
   target_link_libraries(<your-mod-name> mods reborn-patch symbols)
   ```
3. Create `src/main.cpp`:
   ```c++
   #include <libreborn/log.h>

   // Init
   __attribute__((constructor)) static void init_mod() {
       INFO("Loading My New Mod!");
   }
   ```
4. The `init_mod` function (or any others with the `constructor` attribute) will run immediately before the game starts.

## Building And Running
1. Run:
   ```sh
   mkdir build && cd build
   cmake -GNinja ..
   cmake --build .
   ```
2. This should create `lib<your-mod-name>.so` in your newly created build directory.
3. Copy it to the `mods` folder in your profile directory.
4. Run MCPI-Reborn from the terminal, it should now print your log message on startup!
5. Congratulations! You have now created a simple mod!