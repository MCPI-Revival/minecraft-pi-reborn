project(zenity)

# Silence Warnings
add_compile_options(-w)

## Zenity

# Download
add_subdirectory(src EXCLUDE_FROM_ALL)

# Ensure Build
add_custom_target(zenity-build ALL DEPENDS zenity)

# Install
install(TARGETS zenity DESTINATION "${MCPI_BIN_DIR}")
