project(zlib)

# Silence Warnings
add_compile_options(-w)

## zlib

# Download
set(CMAKE_POLICY_DEFAULT_CMP0022 NEW) # Fix Error
add_subdirectory(src EXCLUDE_FROM_ALL)

# Ensure Build
add_custom_target(zlib-build ALL DEPENDS zlibstatic)

# License
install(FILES src/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/zlib")
