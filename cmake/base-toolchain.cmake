# Setup Toolchain
macro(setup_toolchain target)
    # Use ARM Cross-Compiler
    set(CMAKE_C_COMPILER "${target}-gcc")
    set(CMAKE_CXX_COMPILER "${target}-g++")
    set(CMAKE_FIND_ROOT_PATH "/usr/${target}" "/usr/lib/${target}")
    # Extra
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    # pkg-config
    set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/${target}/pkgconfig:/usr/${target}/lib/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig")
endmacro()
