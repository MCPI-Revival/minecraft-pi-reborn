# SDK Support
include("${CMAKE_CURRENT_LIST_DIR}/sdk/util.cmake")

# Setup Headers
function(setup_header_dirs target)
    # Get Header Type
    set(header_type "PUBLIC")
    get_target_property(type "${target}" TYPE)
    if("${type}" STREQUAL "INTERFACE_LIBRARY")
        set(header_type "INTERFACE")
    endif()

    # Add Headers To Target And Install Them
    foreach(dir IN LISTS ARGN)
        target_include_directories("${target}" "${header_type}"
            "$<BUILD_INTERFACE:${dir}>"
        )
        install_sdk_header_dir("${target}" "${dir}")
    endforeach()
    add_sdk_headers("${target}" "${header_type}")
endfunction()

# Export Macros
function(_create_export_macros target)
    # Check Library Type
    get_target_property(type "${target}" TYPE)
    if(NOT "${type}" STREQUAL "SHARED_LIBRARY")
        return()
    endif()

    # Generate
    string(TOUPPER "${target}" macro)
    string(REPLACE "-" "_" macro "${macro}")
    string(PREPEND macro "MCPI_")
    string(APPEND macro "_PUBLIC")
    if(MCPI_WIN32)
        set(export "__declspec(dllexport)")
        set(import "__declspec(dllimport)")
    else()
        set(export "__attribute__((visibility(\"default\")))")
        set(import "${export}")
    endif()

    # Add To Target
    target_compile_definitions("${target}"
        PRIVATE "${macro}=${export}"
        INTERFACE "${macro}=${import}"
    )

    # Set Default Visibility
    set(visibility "hidden")
    set_target_properties("${target}" PROPERTIES
        VISIBILITY_INLINES_HIDDEN TRUE
        C_VISIBILITY_PRESET "${visibility}"
        CXX_VISIBILITY_PRESET "${visibility}"
    )
endfunction()

# Setup & Install Target
function(setup_library target)
    # Arguments
    cmake_parse_arguments(arg
        "INSTALL;IN_SDK;DEFINE_EXPORT_MACRO"
        ""
        ""
        ${ARGN}
    )

    # Install
    if(arg_INSTALL)
        install(TARGETS "${target}"
            RUNTIME DESTINATION "${MCPI_LIB_DIR}"
            LIBRARY DESTINATION "${MCPI_LIB_DIR}" NAMELINK_SKIP
        )
    endif()

    # SDK
    if(arg_IN_SDK)
        setup_library_sdk("${target}")
    endif()

    # Setup Export Macros
    if(arg_DEFINE_EXPORT_MACRO)
        _create_export_macros("${target}")
    endif()
endfunction()