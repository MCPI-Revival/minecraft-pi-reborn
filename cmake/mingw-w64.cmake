# Get Root Directory
execute_process(
    COMMAND "${CMAKE_C_COMPILER}" -print-libgcc-file-name
    OUTPUT_VARIABLE root
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
cmake_path(GET root PARENT_PATH root)

# Install Libraries
file(GLOB files
    LIST_DIRECTORIES FALSE
    "${root}/*.dll"
)
foreach(file IN LISTS files)
    install(
        PROGRAMS "${file}"
        DESTINATION "${MCPI_LIB_DIR}"
    )
endforeach()

# Install Licenses
install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION "${MCPI_LEGAL_DIR}/MinGW-w64"
)