project(zlib)

# Silence Warnings
add_compile_options(-w)

## zlib

# Download
add_subdirectory(src EXCLUDE_FROM_ALL)

# Ensure Build
add_custom_target(zlib-build ALL DEPENDS zlibstatic)
