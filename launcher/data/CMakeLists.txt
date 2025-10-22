project(launcher-data)

# Configure Data
function(configure_data input_name output_var)
    set(configured_path "${CMAKE_CURRENT_BINARY_DIR}/${input_name}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${input_name}" "${configured_path}")
    set("${output_var}" "${configured_path}" PARENT_SCOPE)
endfunction()

# Install Data
function(configure_and_install_data input_name output_path output_name is_desktop_file)
    configure_data("${input_name}" configured_path)
    set(dir "${MCPI_SHARE_DIR}/${output_path}")
    install(
        FILES "${configured_path}"
        DESTINATION "${dir}"
        RENAME "${output_name}"
    )
    if(is_desktop_file)
        set(full_path "${dir}/${output_name}")
        cmake_path(RELATIVE_PATH full_path BASE_DIRECTORY "${MCPI_INSTALL_DIR}")
        target_compile_definitions(launcher-util PRIVATE "DESKTOP_FILE_PATH=\"${full_path}\"")
    endif()
endfunction()

# Configure Metadata
if(NOT MCPI_WIN32)
    # Changelog
    set_property(
        DIRECTORY APPEND PROPERTY
        CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../../docs/CHANGELOG.md"
    )
    execute_process(
        COMMAND
        "${CMAKE_COMMAND}" "-E" "env" "URL=${MCPI_CHANGELOG}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/misc/get-changelog.mjs" "appstream"
        OUTPUT_VARIABLE MCPI_CHANGELOG_XML
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )

    # Linux-Specific
    configure_and_install_data(launcher.desktop applications "${MCPI_APP_ID}.desktop" TRUE)
    configure_and_install_data(appstream.xml metainfo "${MCPI_APP_ID}.appdata.xml" FALSE)
else()
    # Windows-Specific
    configure_data(launcher.manifest manifest_path)
    set(MCPI_MANIFEST "${manifest_path}" PARENT_SCOPE)
endif()