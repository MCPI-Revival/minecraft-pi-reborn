# System Information
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Use The Cross-Compiler
set(target "x86_64-w64-mingw32")
set(CMAKE_C_COMPILER "${target}-gcc")
set(CMAKE_CXX_COMPILER "${target}-g++")
set(CMAKE_RC_COMPILER "${target}-windres")

# Search Paths
set(CMAKE_FIND_ROOT_PATH "/usr/${target}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)