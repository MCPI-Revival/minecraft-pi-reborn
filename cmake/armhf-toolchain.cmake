# Compile For ARM
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64_be" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv8b" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv8l")
    # Force 32-Bit Compile
    add_compile_options("-m32")
elseif(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    # Use ARM Cross-Compiler
    set(TARGET "arm-linux-gnueabihf")
    set(CMAKE_C_COMPILER "${TARGET}-gcc")
    set(CMAKE_CXX_COMPILER "${TARGET}-g++")
    set(CMAKE_FIND_ROOT_PATH "/usr/${TARGET}" "/usr/lib/${TARGET}")
endif()
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "arm")
