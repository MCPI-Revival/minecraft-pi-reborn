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