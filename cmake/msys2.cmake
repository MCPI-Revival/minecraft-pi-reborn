# Get Root Directory
set(root "$ENV{MSYSTEM_PREFIX}")

# Install Libraries
file(GLOB files
    LIST_DIRECTORIES FALSE
    "${root}/bin/*.dll"
)
foreach(file IN LISTS files)
    install(
        PROGRAMS "${file}"
        DESTINATION "${MCPI_LIB_DIR}"
    )
endforeach()

# Install Licenses
file(GLOB projects
    LIST_DIRECTORIES TRUE
    "${root}/share/licenses/*"
)
foreach(project IN LISTS projects)
    install(
        DIRECTORY "${project}"
        DESTINATION "${MCPI_LEGAL_DIR}/MSYS2"
    )
endforeach()