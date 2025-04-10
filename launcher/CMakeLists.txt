project(launcher)

# Launcher
add_executable(launcher
    src/bootstrap/bootstrap.cpp
    src/bootstrap/mods.cpp
    src/bootstrap/assets.cpp
    src/bootstrap/patchelf.cpp
    src/bootstrap/debug.cpp
    src/util/util.cpp
    src/util/sdk.cpp
    src/util/env.cpp
    src/util/install.cpp
    src/logger/logger.cpp
    src/logger/crash-report.cpp
    src/options/parser.cpp
    src/main.cpp
    src/ui/frame.cpp
    src/ui/color.cpp
    src/client/configuration.cpp
    src/client/cache.cpp
    src/client/ui.cpp
    src/updater/updater.cpp
)
target_link_libraries(launcher
    reborn-util
    LIB_LIEF
    imgui
    trampoline-headers
    pthread
)
# RPath
set_target_properties(launcher PROPERTIES INSTALL_RPATH "$ORIGIN/lib/native")
target_link_options(launcher PRIVATE "LINKER:--disable-new-dtags")
# Files
target_compile_definitions(launcher PRIVATE _FILE_OFFSET_BITS=64)

# Install
install(TARGETS launcher DESTINATION "${MCPI_INSTALL_DIR}")
install_symlink("../${MCPI_INSTALL_DIR}/launcher" "bin/${MCPI_APP_NAME}")

# Changelog
set_property(
    DIRECTORY APPEND PROPERTY
    CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../docs/CHANGELOG.md"
)
execute_process(
    COMMAND
        "${CMAKE_COMMAND}" "-E" "env" "URL=${MCPI_CHANGELOG}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/get-changelog.mjs" "appstream"
    OUTPUT_VARIABLE MCPI_CHANGELOG_XML
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

# Data
set(DATA_DIR "data")
set_and_mkdir(CONFIGURED_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/${DATA_DIR}")
function(configure_and_install_data input_name output_path output_name is_desktop_file)
    set(configured_path "${CONFIGURED_DATA_DIR}/${input_name}")
    configure_file("${DATA_DIR}/${input_name}" "${configured_path}")
    set(dir "${MCPI_SHARE_DIR}/${output_path}")
    install(
        FILES "${configured_path}"
        DESTINATION "${dir}"
        RENAME "${output_name}"
    )
    if(is_desktop_file)
        set(full_path "${dir}/${output_name}")
        cmake_path(RELATIVE_PATH full_path BASE_DIRECTORY "${MCPI_INSTALL_DIR}")
        target_compile_definitions(launcher PRIVATE "DESKTOP_FILE_PATH=\"${full_path}\"")
    endif()
endfunction()
configure_and_install_data(launcher.desktop applications "${MCPI_APP_ID}.desktop" TRUE)
configure_and_install_data(appstream.xml metainfo "${MCPI_APP_ID}.appdata.xml" FALSE)

# AppImage
if(MCPI_IS_APPIMAGE_BUILD)
    install_symlink("bin/${MCPI_APP_NAME}" "AppRun")
    install_symlink("${MCPI_SHARE_DIR}/applications/${MCPI_APP_ID}.desktop" "${MCPI_APP_ID}.desktop")
    # Updater
    target_sources(launcher PRIVATE src/updater/appimage.cpp)
endif()
