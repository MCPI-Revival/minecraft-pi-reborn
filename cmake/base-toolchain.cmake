# Sanity Check Return
function(sanity_check_return ret)
    if(NOT ret EQUAL "0")
        message(FATAL_ERROR "Process Failed")
    endif()
endfunction()

# Get Host Architecture
find_program(UNAME uname /bin /usr/bin /usr/local/bin REQUIRED)
execute_process(
    COMMAND "${UNAME}" "-m"
    OUTPUT_VARIABLE HOST_ARCHITECTURE
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE ret
)
sanity_check_return("${ret}")

# Get Include Directories
function(get_include_dirs target compiler result)
    # Get Tool Name
    set(tool "cc1")
    if(compiler MATCHES "^.*g\\+\\+$")
        set(tool "cc1plus")
    endif()

    # Get Tool Path
    execute_process(
        COMMAND "${compiler}" "-print-prog-name=${tool}"
        ERROR_QUIET
 	    OUTPUT_VARIABLE tool
 	    OUTPUT_STRIP_TRAILING_WHITESPACE
 	    RESULT_VARIABLE ret
    )
    sanity_check_return("${ret}")

    # Run Tool To Get Include Path
    set(tool_output "")
    execute_process(
        COMMAND "${tool}" "-quiet" "-v" "-imultiarch" "${target}"
        OUTPUT_QUIET
 	    ERROR_VARIABLE tool_output
 	    ERROR_STRIP_TRAILING_WHITESPACE
 	    INPUT_FILE "/dev/null"
 	    RESULT_VARIABLE ret
    )
    sanity_check_return("${ret}")
    string(REPLACE "\n" ";" tool_output "${tool_output}")

    # Loop
    set(parsing_include_section FALSE)
    foreach(line IN LISTS tool_output)
        # Check Include Section Status
        if(parsing_include_section)
            # Check If Include Section Is Over
            if(line MATCHES "^End of search list.$")
                # Starting Include Section
                set(parsing_include_section FALSE)
                break()
            else()
                # Parsing Include Section
                if(line MATCHES "^ .*$")
                    # Strip Line
                    string(STRIP "${line}" line)
                    # Add To List
                    list(APPEND "${result}" "${line}")
                endif()
            endif()
        else()
            # Check If Include Section Is Starting
            if(line MATCHES "^#include <\\.\\.\\.> search starts here:$")
                # Starting Include Section
                set(parsing_include_section TRUE)
            endif()
        endif()
    endforeach()

    # Return
    set("${result}" "${${result}}" PARENT_SCOPE)
endfunction()

# Setup Include Directories
function(setup_include_dirs compiler target result)
    # Get Full Compiler
    set(full_compiler "${target}-${compiler}")

    # Get Include Directories
    set(include_dirs "")
    get_include_dirs("${target}" "${full_compiler}" include_dirs)

    # Loop
    set(flags "")
    foreach(include_dir IN LISTS include_dirs)
        set(flags "${flags} -isystem ${include_dir}")
    endforeach()

    # Return
    set("${result}" "${${result}} ${flags}" PARENT_SCOPE)
endfunction()

# Setup Toolchain
macro(setup_toolchain target)
    # Use ARM Cross-Compiler
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_C_COMPILER_TARGET "${target}")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_CXX_COMPILER_TARGET "${target}")
    set(CMAKE_FIND_ROOT_PATH "/usr/${target}" "/usr/lib/${target}")
    # Flags
    string(CONCAT NEW_FLAGS
        "-nostdinc "
        "-nostdinc++ "
        "-Wno-unused-command-line-argument"
    )
    set(CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} ${NEW_FLAGS}")
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} ${NEW_FLAGS}")
    # Include Directories
    setup_include_dirs("gcc" "${target}" CMAKE_C_FLAGS_INIT)
    setup_include_dirs("g++" "${target}" CMAKE_CXX_FLAGS_INIT)
    # Extra
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
endmacro()
