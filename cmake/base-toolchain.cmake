# Pick GCC Version
macro(pick_gcc_version gcc_root gcc_version)
    if(NOT DEFINED "${gcc_version}")
        file(GLOB children RELATIVE "${gcc_root}" "${gcc_root}/*")
        set("${gcc_version}" "" CACHE STRING "" FORCE)
        foreach(child IN LISTS children)
            if(IS_DIRECTORY "${gcc_root}/${child}" AND ("${${gcc_version}}" STREQUAL "" OR "${child}" GREATER_EQUAL "${${gcc_version}}"))
                set("${gcc_version}" "${child}" "" CACHE STRING "" FORCE)
            endif()
        endforeach()
        if("${${gcc_version}}" STREQUAL "")
            message(FATAL_ERROR "Unable To Pick GCC Version")
        endif()
        message(STATUS "Using GCC Version: ${${gcc_version}}")
    endif()
endmacro()

# Setup Toolchain
macro(setup_toolchain target)
    # Use ARM Cross-Compiler
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_C_COMPILER_TARGET "${target}")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_CXX_COMPILER_TARGET "${target}")
    set(CMAKE_FIND_ROOT_PATH "/usr/${target}" "/usr/lib/${target}")
    # Include Directories
    pick_gcc_version("/usr/lib/gcc-cross/${target}" GCC_VERSION)
    set(NEW_FLAGS "-nostdinc -nostdinc++ -Wno-unused-command-line-argument -isystem /usr/lib/gcc-cross/${target}/${GCC_VERSION}/include -isystem /usr/${target}/include/c++/${GCC_VERSION} -isystem /usr/${target}/include/c++/${GCC_VERSION}/${target} -isystem /usr/${target}/include")
    set(CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} ${NEW_FLAGS}")
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} ${NEW_FLAGS}")
    # Extra
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
endmacro()
