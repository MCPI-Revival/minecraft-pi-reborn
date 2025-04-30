cmake_minimum_required(VERSION 3.25.0)

# Utility Functions
include(../cmake/util/util.cmake)

# Force Build Type
force_set(CMAKE_BUILD_TYPE "Release" STRING)

# Project
project(tools)

# Options
force_set(BUILD_TESTING FALSE BOOL)
force_set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/bin" FILEPATH)
force_set(CMAKE_INSTALL_BINDIR "." FILEPATH)

# Versions
set(NINJA_VERSION "d4a4f3694c983306584b26c8e45bf3efcd53d108")
set(APPIMAGETOOL_VERSION "continuous")
set(APPSTREAM_VERSION "v1.0.4")

# Ninja
include(FetchContent)
FetchContent_Declare(ninja
    GIT_REPOSITORY "https://github.com/digit-google/ninja.git"
    GIT_TAG "${NINJA_VERSION}"
)
FetchContent_MakeAvailable(ninja)
install(PROGRAMS "${ninja_SOURCE_DIR}/misc/jobserver_pool.py" TYPE BIN)

# Download AppImageTool
set(APPIMAGETOOL_NAME "appimagetool")
set(APPIMAGETOOL_BIN "${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGETOOL_NAME}.AppImage")
get_arch(ARCH)
safe_download(
    "AppImageTool"
    "https://github.com/AppImage/appimagetool/releases/download/${APPIMAGETOOL_VERSION}/appimagetool-${ARCH}.AppImage"
    "${APPIMAGETOOL_BIN}"
)
# Fix
execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/fix-appimage-for-docker.sh" "${APPIMAGETOOL_BIN}"
    COMMAND_ERROR_IS_FATAL ANY
)
# Install
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGETOOL_NAME}"
    "#!/bin/sh\n"
    "exec \"${APPIMAGETOOL_BIN}\" --appimage-extract-and-run \"\$@\"\n"
)
install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGETOOL_NAME}" TYPE BIN)

# AppStream
include(ExternalProject)
ExternalProject_Add(appstream
    GIT_REPOSITORY "https://github.com/ximion/appstream.git"
    GIT_TAG "${APPSTREAM_VERSION}"
    # Patch
    PATCH_COMMAND sed -i "/i18n\\|po\\/\\|data\\/\\|docs\\/\\|tests\\//d" meson.build
    # Configure
    CONFIGURE_COMMAND
        "meson" "setup"
        "--prefix=<INSTALL_DIR>"
        "--buildtype=release"
        "--default-library=static"
        "-Dsystemd=false"
        "-Dstemming=false"
        "-Dgir=false"
        "<BINARY_DIR>" "<SOURCE_DIR>"
    CONFIGURE_HANDLED_BY_BUILD TRUE
    USES_TERMINAL_CONFIGURE TRUE
    # Build
    BUILD_COMMAND "meson" "compile"
    USES_TERMINAL_BUILD TRUE
    # Install
    INSTALL_COMMAND "meson" "install" "--quiet"
    USES_TERMINAL_INSTALL TRUE
)
ExternalProject_Get_Property(appstream INSTALL_DIR)
install(PROGRAMS "${INSTALL_DIR}/bin/appstreamcli" TYPE BIN)