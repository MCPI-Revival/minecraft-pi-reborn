project(images)

# Logo
install(
    FILES "modern_logo.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/gui"
)

# Title Background
install(
    FILES "background.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/gui"
    RENAME "titleBG.png"
)

# Mojang Textures
install(
    FILES "mojang/chest.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/item"
)
install(
    FILES "mojang/shadow.png" "mojang/vignette.png" "mojang/grasscolor.png"
    DESTINATION "${MCPI_INSTALL_DIR}/data/images/misc"
)

# Icon
set(ICON_FILE "icon.png")
set(ICON_DIR "${MCPI_SHARE_DIR}/icons/hicolor/512x512/apps")
set(ICON_NAME "${MCPI_APP_ID}.png")
install(
    FILES "${ICON_FILE}"
    DESTINATION "${ICON_DIR}"
    RENAME "${ICON_NAME}"
)
set(ICON_PATH "${ICON_DIR}/${ICON_NAME}")
cmake_path(RELATIVE_PATH ICON_PATH BASE_DIRECTORY "${MCPI_INSTALL_DIR}")
target_compile_definitions(launcher PRIVATE
    "ICON_PATH=\"${ICON_PATH}\""
)

# Icon (ICO)
set(ICO_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${ICON_FILE}")
get_target_property(ICO_OUTPUT launcher-meta ICO_PATH)
add_custom_command(OUTPUT "${ICO_OUTPUT}"
    DEPENDS "${ICO_INPUT}"
    COMMAND
        "magick" "convert"
        "${ICO_INPUT}"
        "-define" "icon:auto-resize=16,32,48,64,256"
        "${ICO_OUTPUT}"
    VERBATIM
)
add_library(icon-ico INTERFACE
    "${ICO_OUTPUT}"
)
target_link_libraries(launcher-meta INTERFACE
    icon-ico
)

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    install_symlink("${ICON_DIR}/${MCPI_APP_ID}.png" "${MCPI_APP_ID}.png")
    install_symlink("${MCPI_APP_ID}.png" ".DirIcon")
endif()
