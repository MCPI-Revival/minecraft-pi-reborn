# I/O Functions
include("${CMAKE_CURRENT_LIST_DIR}/io.cmake")

# Embed Resources
set(util_list_dir "${CMAKE_CURRENT_LIST_DIR}")
function(embed_resource target file)
    # Input
    set(in "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    set_property(
        DIRECTORY APPEND PROPERTY
        CMAKE_CONFIGURE_DEPENDS "${in}"
    )
    # Output
    cmake_path(GET file FILENAME name)
    string(MAKE_C_IDENTIFIER "${name}" name)
    set(out "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
    target_sources("${target}" PRIVATE "${out}")

    # Read File
    file(READ "${in}" data HEX)
    # Convert Hex Data For C Compatibility
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," data "${data}")

    # Write Data
    configure_file("${util_list_dir}/resource.c.in" "${out}")
endfunction()

# Nicer Output
function(message log_level)
    if((NOT MESSAGE_QUIET) OR (NOT (log_level STREQUAL "STATUS" OR log_level MATCHES "^CHECK_")))
        _message("${log_level}" ${ARGN})
    endif()
endfunction()

# Force Set Configuration Variable
string(CONCAT FORCE_SET_FUNCTION
    "function(force_set name value type)\n"
    "    set(\"\${name}\" \"\${value}\" CACHE \"\${type}\" \"\" FORCE)\n"
    "    mark_as_advanced(FORCE \"\${name}\")\n"
    "endfunction()\n"
)
cmake_language(EVAL CODE "${FORCE_SET_FUNCTION}")

# Get Build Computer Architecture
function(get_arch var)
    execute_process(COMMAND uname -m OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(arch STREQUAL "armv8b" OR arch STREQUAL "armv8l")
        set(arch "aarch64")
    endif()
    set("${var}" "${arch}" PARENT_SCOPE)
endfunction()