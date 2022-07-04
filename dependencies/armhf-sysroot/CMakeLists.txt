project(armhf-sysroot)

# Check If Using Custom Sysroot
if(MCPI_CUSTOM_BUNDLED_ARMHF_SYSROOT)
    # Custom Sysroot
    set(SYSROOT_DIR "${MCPI_CUSTOM_BUNDLED_ARMHF_SYSROOT}")
else()
    # Download From APT
    set(APT_PACKAGES "libc6" "libstdc++6")

    # Copy To Binary Directory
    set(APT_DIR "${CMAKE_CURRENT_BINARY_DIR}/apt")
    file(MAKE_DIRECTORY "${APT_DIR}")
    # Make Directories
    file(MAKE_DIRECTORY "${APT_DIR}/apt.conf.d")
    file(MAKE_DIRECTORY "${APT_DIR}/preferences.d")
    file(MAKE_DIRECTORY "${APT_DIR}/keys")
    file(MAKE_DIRECTORY "${APT_DIR}/dpkg")
    file(MAKE_DIRECTORY "${APT_DIR}/archives")
    file(TOUCH "${APT_DIR}/dpkg/status")

    # Create APT Sources
    execute_process(
        COMMAND "${CMAKE_SOURCE_DIR}/scripts/tools/get-apt-sources.sh"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE APT_SOURCES
        RESULT_VARIABLE APT_SOURCES_RESULT
    )
    if(NOT APT_SOURCES_RESULT EQUAL 0)
        message(FATAL_ERROR "Unable To Get APT Sources For ARMHF Sysroot")
    endif()
    file(WRITE "${APT_DIR}/sources.list" "${APT_SOURCES}")

    # Create APT Config
    string(CONCAT APT_CONFIG
        "Dir \"${APT_DIR}\";\n"
        "Dir::State \"${APT_DIR}\";\n"
        "Dir::Cache \"${APT_DIR}\";\n"
        "Dir::Etc::Main \"${APT_DIR}/apt.conf\";\n"
        "Dir::Etc::Parts \"${APT_DIR}/apt.conf.d\";\n"
        "Dir::Etc::SourceList \"${APT_DIR}/sources.list\";\n"
        "Dir::Etc::SourceListParts \"${APT_DIR}/sources.list.d\";\n"
        "Dir::Etc::PreferencesParts \"${APT_DIR}/preferences.d\";\n"
        "Dir::Etc::TrustedParts \"${APT_DIR}/keys\";\n"
        "Dir::State::status \"${APT_DIR}/dpkg/status\";\n"
        "Dir::Ignore-Files-Silently \"False\";\n"
        "APT::Install-Recommends \"False\";\n"
        "APT::Install-Suggests \"False\";\n"
        "APT::Immediate-Configure \"False\";\n"
        "APT::Architecture \"armhf\";\n"
        "APT::Architectures { \"armhf\"; }\n"
        "Acquire::Languages \"none\";\n"
        "APT::Get::AllowUnauthenticated \"True\";\n"
        "Acquire::AllowInsecureRepositories \"True\";\n"
    )
    file(WRITE "${APT_DIR}/apt.conf" "${APT_CONFIG}")

    # Environment
    set(APT_ENV
        "${CMAKE_COMMAND}"
        -E env
        "DEBIAN_FRONTEND=noninteractive"
        "APT_CONFIG=${APT_DIR}/apt.conf"
    )

    # Create Sysroot Directory
    set(SYSROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/sysroot")
    file(MAKE_DIRECTORY "${SYSROOT_DIR}")

    # Download
    add_custom_command(
        OUTPUT "${APT_DIR}/.update-stamp"
        COMMAND ${APT_ENV} apt-get update
        COMMAND "${CMAKE_COMMAND}" -E touch "${APT_DIR}/.update-stamp"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${APT_DIR}/.download-stamp"
        DEPENDS "${APT_DIR}/.update-stamp"
        COMMAND find "${APT_DIR}/archives" -maxdepth 1 -type f -name "*.deb" -delete
        COMMAND ${APT_ENV} apt-get install -y --no-install-recommends --download-only ${APT_PACKAGES}
        COMMAND "${CMAKE_COMMAND}" -E touch "${APT_DIR}/.download-stamp"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${APT_DIR}/.extract-stamp"
        DEPENDS "${APT_DIR}/.download-stamp"
        COMMAND find "${SYSROOT_DIR}" -mindepth 1 -delete
        COMMAND find "${APT_DIR}/archives" -maxdepth 1 -type f -name "*.deb" -exec dpkg -x {} "${SYSROOT_DIR}" ";"
        COMMAND "${CMAKE_COMMAND}" -E touch "${APT_DIR}/.extract-stamp"
        VERBATIM
    )
    add_custom_target(armhf-sysroot ALL DEPENDS "${APT_DIR}/.extract-stamp")
endif()

# Install
install(
    DIRECTORY "${SYSROOT_DIR}/"
    DESTINATION "${MCPI_INSTALL_DIR}/sysroot"
    USE_SOURCE_PERMISSIONS
    REGEX "usr/lib/arm-linux-gnueabihf/gconv" EXCLUDE
    REGEX "usr/lib/arm-linux-gnueabihf/audit" EXCLUDE
    REGEX "usr/share/man" EXCLUDE
    REGEX "usr/share/doc/.*/README\..*" EXCLUDE
    REGEX "usr/share/doc/.*/changelog\..*" EXCLUDE
    REGEX "usr/share/doc/.*/NEWS\..*" EXCLUDE
    REGEX "usr/share/doc/.*/TODO\..*" EXCLUDE
    REGEX "usr/share/lintian" EXCLUDE
    REGEX "usr/share/gcc" EXCLUDE
    REGEX "usr/share/gdb" EXCLUDE
    REGEX "usr/share/locale" EXCLUDE
    REGEX "usr/share/help" EXCLUDE
    REGEX "etc" EXCLUDE
)
