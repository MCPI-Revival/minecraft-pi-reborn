project(patchelf)

# Silence Warnings
add_compile_options(-w)

## PatchELF

# Build
add_executable(patchelf src/src/patchelf.cc)
target_compile_definitions(patchelf PRIVATE -D_FILE_OFFSET_BITS=64)
set_target_properties(patchelf PROPERTIES CXX_STANDARD 17)

# Install
install(TARGETS patchelf DESTINATION "${MCPI_BIN_DIR}")

# License
install(FILES src/COPYING DESTINATION "${MCPI_LEGAL_DIR}/patchelf")
