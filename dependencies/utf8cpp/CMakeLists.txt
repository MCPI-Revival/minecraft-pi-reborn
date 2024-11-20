project(utf8cpp)

# Silence Warnings
add_compile_options(-w)

## stb_image

# Build
add_subdirectory(src EXCLUDE_FROM_ALL)

# License
install(FILES src/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/UTF8-CPP")
