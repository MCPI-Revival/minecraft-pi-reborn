# Get Root Directory
execute_process(
    COMMAND "${CMAKE_C_COMPILER}" -print-libgcc-file-name
    OUTPUT_VARIABLE root
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
cmake_path(GET root PARENT_PATH root)

# Install Libraries
install(
    PROGRAMS
        "${root}/libgcc_s_seh-1.dll"
        "${root}/libstdc++-6.dll"
        "/usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll"
    DESTINATION "${MCPI_LIB_DIR}"
)

# Install Licenses
install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION "${MCPI_LEGAL_DIR}/MinGW-w64"
)