# Get Root Directory
set(dir "$ENV{MSYSTEM_PREFIX}/bin")

# Get Libraries
file(GLOB files
    LIST_DIRECTORIES FALSE
    "${dir}/*.dll"
)

# Install Libraries
foreach(file IN LISTS files)
    install(
        PROGRAMS "${file}"
        DESTINATION "${MCPI_LIB_DIR}"
    )
endforeach()