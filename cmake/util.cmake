# Symlink Function
function(install_symlink target link)
    get_filename_component(parent "${link}" DIRECTORY)
    if(parent STREQUAL "")
        set(parent ".")
    endif()
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/symlink/${parent}")
    file(CREATE_LINK "${target}" "${CMAKE_BINARY_DIR}/symlink/${link}" SYMBOLIC)
    install(FILES "${CMAKE_BINARY_DIR}/symlink/${link}" DESTINATION "${parent}")
endfunction()

# Embed Resources
function(embed_resource target file)
    # Read Hex Data
    file(READ "${file}" data HEX)
    # Convert Hex Data For C Compatibility
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," data "${data}")
    # Get C Name
    get_filename_component(name "${file}" NAME)
    string(MAKE_C_IDENTIFIER "${name}" name)
    # Write Data
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${name}.c" "#include <stddef.h>\nconst unsigned char ${name}[] = {${data}};\nconst size_t ${name}_len = sizeof (${name});\n")
    # Add To Target
    target_sources("${target}" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
    # Mark Dependency
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${file}"
    )
endfunction()
