project(stb_image)

# Silence Warnings
add_compile_options(-w)

## stb_image

# Build
add_library(stb_image SHARED src/impl.cpp)
target_link_libraries(stb_image PRIVATE m)

# Headers
set(in "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(out "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY "${out}")
function(configure_header header)
    file(READ "${in}/original/${header}" data)
    configure_file("${in}/${header}.in" "${out}/${header}" @ONLY)
endfunction()
configure_header(stb_image.h)
configure_header(stb_image_write.h)
setup_header_dirs(stb_image "${out}")

# Install
setup_library(stb_image
    INSTALL
    IN_SDK
    DEFINE_EXPORT_MACRO
)

# License
install(FILES include/original/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/stb_image")
