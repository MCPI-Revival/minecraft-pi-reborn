project(zenity)

# Silence Warnings
add_compile_options(-w)

## Zenity

# Download
set(MESSAGE_QUIET TRUE)
add_subdirectory(src EXCLUDE_FROM_ALL)
unset(MESSAGE_QUIET)

# Ensure Build
add_custom_target(zenity-build ALL DEPENDS zenity)

# Install
install(TARGETS zenity DESTINATION "${MCPI_BIN_DIR}")

# License
install(FILES src/COPYING DESTINATION "${MCPI_LEGAL_DIR}/zenity")
