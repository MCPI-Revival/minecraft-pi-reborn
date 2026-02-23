# Set Compiler
set(CMAKE_C_COMPILER "${target}-gcc")
set(CMAKE_CXX_COMPILER "${target}-g++")

# Custom Search Paths
set(CMAKE_FIND_ROOT_PATH
    "/usr/${target}"
    "/usr/lib/${target}"
    "/usr"
)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)