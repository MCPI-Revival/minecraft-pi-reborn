# Setup Toolchain
macro(setup_toolchain target)
    # Target Variants
    set(target_variants "${target}")
    macro(add_target_variant value)
        string(REPLACE "-linux" "-${value}-linux" target_variant "${target}")
        list(APPEND target_variants "${target_variant}")
    endmacro()
    add_target_variant(unknown)
    add_target_variant(none)
    add_target_variant(pc)
    # Find Compiler
    macro(find_compiler output name)
        set(possible_names "")
        foreach(possible_target IN LISTS target_variants)
            list(APPEND possible_names "${possible_target}-${name}")
        endforeach()
        find_program(
            "${output}"
            NAMES ${possible_names}
            NO_CACHE
        )
        if("${${output}}" STREQUAL "${output}-NOTFOUND")
            message(FATAL_ERROR "Unable To Find ${name}")
        endif()
    endmacro()
    find_compiler(CMAKE_C_COMPILER "gcc")
    find_compiler(CMAKE_CXX_COMPILER "g++")
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
