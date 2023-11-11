project(stb_image)

# Silence Warnings
add_compile_options(-w)

## stb_image

# Build
add_library(stb_image SHARED src/stb_image_impl.c)
target_include_directories(
    stb_image
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${MCPI_SDK_INCLUDE_DIR}/stb_image>"
)
target_link_libraries(stb_image PRIVATE m)
target_compile_definitions(stb_image PUBLIC STBI_ONLY_PNG)

# Install
install(TARGETS stb_image DESTINATION "${MCPI_LIB_DIR}")
install(
    DIRECTORY "include/"
    DESTINATION "${MCPI_SDK_INCLUDE_DIR}/stb_image"
    FILES_MATCHING
    PATTERN "*.h"
)
install(TARGETS stb_image EXPORT sdk DESTINATION "${MCPI_SDK_LIB_DIR}")

# License
install(FILES include/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/stb_image")
