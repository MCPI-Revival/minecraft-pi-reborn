# Setup Toolchain
macro(setup_toolchain target)
    # Use ARM Cross-Compiler
    set(CMAKE_C_COMPILER "${target}-gcc")
    set(CMAKE_CXX_COMPILER "${target}-g++")
    # Extra
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    # Custom Search Paths
    if(NOT DEFINED ENV{MCPI_TOOLCHAIN_USE_DEFAULT_SEARCH_PATHS})
        # Find Root
        set(CMAKE_FIND_ROOT_PATH "/usr/${target}" "/usr/lib/${target}")
        # pkg-config
        set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/${target}/pkgconfig:/usr/${target}/lib/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig")
    endif()
endmacro()
