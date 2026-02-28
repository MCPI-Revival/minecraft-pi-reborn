# Where To Store Headers For Target
macro(_get_sdk_header_dir target)
    if(NOT BUILD_ARM_COMPONENTS)
        return()
    endif()
    set(sdk_dir "${MCPI_SDK_INCLUDE_DIR}/${target}")
endmacro()

# Setup Header Directories For Target
function(install_sdk_header_dir target dir)
    # Install Headers
    _get_sdk_header_dir("${target}")
    install(
        DIRECTORY "${dir}/"
        DESTINATION "${sdk_dir}"
    )
endfunction()
function(add_sdk_headers target header_type)
    # Add SDK Headers To SDK
    _get_sdk_header_dir("${target}")
    target_include_directories("${target}" "${header_type}"
        "$<INSTALL_INTERFACE:${sdk_dir}>"
    )
endfunction()

# Setup & Install Target
function(setup_library_sdk target)
    install(TARGETS "${target}" EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")
endfunction()