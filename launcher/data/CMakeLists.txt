project(launcher-data)

# Configure Data
function(configure_data input_path output_var)
    cmake_path(GET input_path FILENAME input_name)
    set(configured_path "${CMAKE_CURRENT_BINARY_DIR}/${input_name}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${input_path}" "${configured_path}")
    set("${output_var}" "${configured_path}" PARENT_SCOPE)
endfunction()

# Install Data
function(configure_and_install_data input_path output_path output_name)
    configure_data("${input_path}" configured_path)
    set(dir "${MCPI_SHARE_DIR}/${output_path}")
    install(
        FILES "${configured_path}"
        DESTINATION "${dir}"
        RENAME "${output_name}"
    )
    set(full_path "${dir}/${output_name}" PARENT_SCOPE)
endfunction()

# Read Changelog
set_property(
    DIRECTORY APPEND PROPERTY
    CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../../docs/CHANGELOG.md"
)
macro(read_changelog mode out)
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" "-E" "env" "URL=${MCPI_CHANGELOG}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/misc/get-changelog.mjs" "${mode}"
        OUTPUT_VARIABLE "${out}"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
endmacro()

# Metadata Attached To The Binaries
add_library(launcher-meta INTERFACE)
read_changelog(date MCPI_VERSION_DATE)
target_compile_definitions(launcher-meta INTERFACE
    "RELEASE_DATE_STR=\"Released ${MCPI_VERSION_DATE}\""
)

# Configure Metadata
if(NOT MCPI_WIN32)
    # AppStream
    read_changelog(appstream MCPI_CHANGELOG_XML)
    string(REPLACE "&" "&amp;" MCPI_AUTHOR_LONG_SAFE "${MCPI_AUTHOR_LONG}")
    configure_and_install_data(linux/appstream.xml metainfo "${MCPI_APP_ID}.appdata.xml")

    # Desktop Entry
    configure_and_install_data(linux/launcher.desktop applications "${MCPI_APP_ID}.desktop")
    if(MCPI_IS_APPIMAGE_BUILD)
        install_symlink("${full_path}" "${MCPI_APP_ID}.desktop")
    endif()
    cmake_path(RELATIVE_PATH full_path BASE_DIRECTORY "${MCPI_INSTALL_DIR}")
    target_compile_definitions(launcher-meta INTERFACE
        "DESKTOP_FILE_PATH=\"${full_path}\""
    )
else()
    # Icon
    set(MCPI_ICO_PATH "${CMAKE_CURRENT_BINARY_DIR}/icon.ico")
    set_target_properties(launcher-meta PROPERTIES
        ICO_PATH "${MCPI_ICO_PATH}"
    )
    set(resources "${MCPI_ICO_PATH}")

    # Manifest
    set(MCPI_VERSION_WIN32 "${MCPI_VERSION}.0")
    configure_data(win32/launcher.manifest MCPI_MANIFEST_PATH)
    list(APPEND resources "${MCPI_MANIFEST_PATH}")

    # Resources
    string(REPLACE "." "," MCPI_VERSION_RC "${MCPI_VERSION_WIN32}")
    configure_data(win32/launcher.rc MCPI_RC_PATH)
    target_sources(launcher-meta INTERFACE
        "${MCPI_RC_PATH}"
    )
    set_source_files_properties("${MCPI_RC_PATH}"
        DIRECTORY ..
        PROPERTIES OBJECT_DEPENDS "${resources}"
    )
    target_compile_definitions(launcher-meta INTERFACE
        "$<$<AND:$<COMPILE_LANGUAGE:RC>,$<CONFIG:Debug>>:VER_DEBUG>"
    )
endif()