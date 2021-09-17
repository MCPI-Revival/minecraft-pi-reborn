# Compile For ARM
if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64_be" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "armv8b" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "armv8l")
    # Force 32-Bit Compile
    add_compile_options("-m32")
elseif(NOT CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm")
    # Use ARM Cross-Compiler
    include("${CMAKE_CURRENT_LIST_DIR}/base-toolchain.cmake")
    setup_toolchain("arm-linux-gnueabihf")
endif()
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "arm")
