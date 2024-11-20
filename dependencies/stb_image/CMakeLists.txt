project(stb_image)

# Silence Warnings
add_compile_options(-w)

## stb_image

# Build
add_library(stb_image SHARED src/stb_image_impl.c)
target_link_libraries(stb_image PRIVATE m)
target_compile_definitions(stb_image PUBLIC STBI_ONLY_PNG)
setup_header_dirs(stb_image "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Install
setup_library(stb_image TRUE TRUE)

# License
install(FILES include/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/stb_image")
