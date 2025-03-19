# Symlink Function
function(install_symlink target link)
    cmake_path(GET link PARENT_PATH parent)
    if(parent STREQUAL "")
        set(parent ".")
    endif()
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/symlink/${parent}")
    file(CREATE_LINK "${target}" "${CMAKE_BINARY_DIR}/symlink/${link}" SYMBOLIC)
    install(FILES "${CMAKE_BINARY_DIR}/symlink/${link}" DESTINATION "${parent}")
endfunction()

# Embed Resources
set(util_list_dir "${CMAKE_CURRENT_LIST_DIR}")
function(embed_resource target file)
    # Get C Name
    cmake_path(GET file FILENAME name)
    string(MAKE_C_IDENTIFIER "${name}" name)
    # Add Command
    set(in "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    set(out "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
    set(script "${util_list_dir}/embed-resource.cmake")
    add_custom_command(OUTPUT "${out}"
        COMMAND "${CMAKE_COMMAND}"
        ARGS "-DEMBED_IN=${in}" "-DEMBED_OUT=${out}" "-P" "${script}"
        DEPENDS "${in}" "${script}"
        VERBATIM
    )
    # Add To Target
    target_sources("${target}" PRIVATE "${out}")
endfunction()

# Nicer Output
function(message log_level)
    if((NOT MESSAGE_QUIET) OR (NOT (log_level STREQUAL "STATUS" OR log_level MATCHES "^CHECK_")))
        _message("${log_level}" ${ARGN})
    endif()
endfunction()

# Exporting Targets And Headers
macro(_get_sdk_header_dir target)
    set(sdk_dir "${MCPI_SDK_INCLUDE_DIR}/${target}")
endmacro()
function(setup_header_dirs target)
    _get_sdk_header_dir("${target}")
    # Get Header Type
    set(header_type "PUBLIC")
    get_target_property(type "${target}" TYPE)
    if ("${type}" STREQUAL "INTERFACE_LIBRARY")
        set(header_type "INTERFACE")
    endif()
    # Loop
    foreach(dir IN LISTS ARGN)
        # Add To Target
        target_include_directories("${target}" "${header_type}" "$<BUILD_INTERFACE:${dir}>")
        # Add To SDK
        if(BUILD_ARM_COMPONENTS)
            install(
                DIRECTORY "${dir}/"
                DESTINATION "${sdk_dir}"
                FILES_MATCHING
                PATTERN "*.h"
            )
        endif()
    endforeach()
    # Add SDK Headers To Target
    if(BUILD_ARM_COMPONENTS)
        target_include_directories("${target}" "${header_type}" "$<INSTALL_INTERFACE:${sdk_dir}>")
    endif()
endfunction()
function(setup_library target should_install should_export)
    # Install
    if(should_install)
        install(TARGETS "${target}" DESTINATION "${MCPI_LIB_DIR}")
    endif()
    # SDK
    if(should_export AND BUILD_ARM_COMPONENTS)
        install(TARGETS "${target}" EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
    endif()
endfunction()

# Force Set Configuration Variable
function(force_set name value type)
    set("${name}" "${value}" CACHE "${type}" "" FORCE)
    mark_as_advanced(FORCE "${name}")
endfunction()

# Make Directory
function(set_and_mkdir name dir)
    set("${name}" "${dir}" PARENT_SCOPE)
    file(MAKE_DIRECTORY "${dir}")
endfunction()

# Download File With Error-Checking
function(safe_download name url out)
    file(DOWNLOAD
        "${url}"
        "${out}"
        STATUS status
    )
    list(GET status 0 status_code)
    list(GET status 1 error_message)
    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "Unable To Download ${name}: ${error_message}")
    else()
        message(STATUS "Downloaded ${name}: ${out}")
    endif()
endfunction()