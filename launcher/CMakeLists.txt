project(launcher)

# Launcher
add_executable(launcher
    src/bootstrap.cpp
    src/patchelf.cpp
    src/util.cpp
    src/crash-report.cpp
    src/sdk.cpp
    src/mods.cpp
    src/options/parser.cpp
    src/main.cpp
    src/client/configuration.cpp
    src/client/cache.cpp
    src/client/available-feature-flags # Show In IDE
)
embed_resource(launcher src/client/available-feature-flags)
target_link_libraries(launcher reborn-util LIB_LIEF trampoline-headers)
# RPath
set_target_properties(launcher PROPERTIES INSTALL_RPATH "$ORIGIN/lib/native")
target_link_options(launcher PRIVATE "LINKER:--disable-new-dtags")

# Install
install(TARGETS launcher DESTINATION "${MCPI_INSTALL_DIR}")
install_symlink("../${MCPI_INSTALL_DIR}/launcher" "bin/${MCPI_VARIANT_NAME}")

# Install Desktop Entry
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/launcher.desktop"
    "[Desktop Entry]\n"
    "Name=${MCPI_APP_TITLE}\n"
    "Comment=Fun with Blocks\n"
    "Icon=${MCPI_APP_ID}\n"
    "Exec=${MCPI_VARIANT_NAME}\n"
    "Type=Application\n"
    "Categories=Game;\n"
)
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/launcher.desktop"
    "Terminal=false\n"
    "StartupNotify=false\n"
    "StartupWMClass=${MCPI_APP_ID}\n"
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/launcher.desktop"
    DESTINATION "${MCPI_SHARE_DIR}/applications"
    RENAME "${MCPI_APP_ID}.desktop"
)

# Install AppStream Metadata
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/appstream.xml"
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<component type=\"desktop\">\n"
    "    <id>${MCPI_APP_ID}</id>\n"
    "    <name>${MCPI_APP_TITLE}</name>\n"
    "    <metadata_license>CC0-1.0</metadata_license>\n"
    "    <summary>Fun with Blocks</summary>\n"
    "    <description>\n"
    "        <p>Minecraft: Pi Edition Modding Project.</p>\n"
    "        <p>NOTE: This is not verified by, affiliated with, or supported by Mojang or Microsoft.</p>\n"
    "    </description>\n"
    "    <url type=\"homepage\">https://gitea.thebrokenrail.com/TheBrokenRail/minecraft-pi-reborn</url>\n"
    "    <launchable type=\"desktop-id\">${MCPI_APP_ID}.desktop</launchable>\n"
    "    <provides>\n"
    "        <id>com.thebrokenrail.MCPIRebornClient.desktop</id>\n"
    "    </provides>\n"
    "    <project_license>LicenseRef-proprietary</project_license>\n"
    "    <developer_name>TheBrokenRail &amp; Mojang AB</developer_name>\n"
    "    <content_rating type=\"oars-1.0\">\n"
    "        <content_attribute id=\"violence-cartoon\">moderate</content_attribute>\n"
    "        <content_attribute id=\"violence-fantasy\">none</content_attribute>\n"
    "        <content_attribute id=\"violence-realistic\">none</content_attribute>\n"
    "        <content_attribute id=\"violence-bloodshed\">none</content_attribute>\n"
    "        <content_attribute id=\"violence-sexual\">none</content_attribute>\n"
    "        <content_attribute id=\"drugs-alcohol\">none</content_attribute>\n"
    "        <content_attribute id=\"drugs-narcotics\">none</content_attribute>\n"
    "        <content_attribute id=\"drugs-tobacco\">none</content_attribute>\n"
    "        <content_attribute id=\"sex-nudity\">none</content_attribute>\n"
    "        <content_attribute id=\"sex-themes\">none</content_attribute>\n"
    "        <content_attribute id=\"language-profanity\">none</content_attribute>\n"
    "        <content_attribute id=\"language-humor\">none</content_attribute>\n"
    "        <content_attribute id=\"language-discrimination\">none</content_attribute>\n"
    "        <content_attribute id=\"social-chat\">intense</content_attribute>\n"
    "        <content_attribute id=\"social-info\">none</content_attribute>\n"
    "        <content_attribute id=\"social-audio\">none</content_attribute>\n"
    "        <content_attribute id=\"social-location\">none</content_attribute>\n"
    "        <content_attribute id=\"social-contacts\">none</content_attribute>\n"
    "        <content_attribute id=\"money-purchasing\">none</content_attribute>\n"
    "        <content_attribute id=\"money-gambling\">none</content_attribute>\n"
    "    </content_rating>\n"
    "    <releases>\n"
    "        <release version=\"${MCPI_VERSION}\" date=\"${MCPI_VERSION_DATE}\"></release>\n"
    "    </releases>\n"
    "    <screenshots>\n"
    "        <screenshot type=\"default\">\n"
    "            <image>https://gitea.thebrokenrail.com/TheBrokenRail/minecraft-pi-reborn/raw/branch/master/images/start.png</image>\n"
    "        </screenshot>\n"
    "    </screenshots>\n"
    "</component>\n"
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/appstream.xml"
    DESTINATION "${MCPI_SHARE_DIR}/metainfo"
    RENAME "${MCPI_APP_ID}.appdata.xml"
)

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    install_symlink("bin/${MCPI_VARIANT_NAME}" "AppRun")
    install_symlink("${MCPI_SHARE_DIR}/applications/${MCPI_APP_ID}.desktop" "${MCPI_APP_ID}.desktop")
endif()
