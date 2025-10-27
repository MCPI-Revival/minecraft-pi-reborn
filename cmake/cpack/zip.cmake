# Utility Function
macro(get_files dir)
    file(GLOB files
        LIST_DIRECTORIES TRUE
        RELATIVE "${dir}"
        "${dir}/*"
    )
endmacro()

# Prepare
set(root "${CPACK_TEMPORARY_DIRECTORY}")
get_files("${root}")
set(tmp "${root}/tmp")
file(MAKE_DIRECTORY "${tmp}")
foreach(file IN LISTS files)
    file(RENAME
        "${root}/${file}"
        "${tmp}/${file}"
    )
endforeach()

# Move Files
set(dir "${tmp}/${CPACK_MCPI_INSTALL_DIR}")
get_files("${dir}")
foreach(file IN LISTS files)
    file(RENAME
        "${dir}/${file}"
        "${root}/${file}"
    )
endforeach()

# Clean Up
file(REMOVE_RECURSE "${tmp}")