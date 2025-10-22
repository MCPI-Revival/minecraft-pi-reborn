# Where To Store Headers For Target
macro(_get_sdk_header_dir target)
    set(sdk_dir "${MCPI_SDK_INCLUDE_DIR}/${target}")
endmacro()

# Setup Header Directories For Target
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

# Setup & Install Target
function(setup_library target should_install should_export)
    # Install
    if(should_install)
        install(TARGETS "${target}" RUNTIME LIBRARY DESTINATION "${MCPI_LIB_DIR}")
    endif()
    # SDK
    if(should_export AND BUILD_ARM_COMPONENTS)
        install(TARGETS "${target}" EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
    endif()
endfunction()