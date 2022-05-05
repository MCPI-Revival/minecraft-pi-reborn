#!/bin/sh

set -e

# ARM Toolchain File
ARM_TOOLCHAIN_FILE="$(pwd)/cmake/armhf-toolchain.cmake"

# Setup
setup() {
    # Find Toolchain
    local toolchain_file="$(pwd)/cmake/${ARCH}-toolchain.cmake"
    if [ ! -f "${toolchain_file}" ]; then
        echo "Invalid Architecture: ${ARCH}" > /dev/stderr
        exit 1
    fi

    # Create Build Dir
    rm -rf "build/${MODE}-${ARCH}"
    mkdir -p "build/${MODE}-${ARCH}"
    cd "build/${MODE}-${ARCH}"

    # Prepare
    local server_mode='OFF'
    if [ "${MODE}" = "server" ]; then
        server_mode='ON'
    fi

    # Build ARM Components
    mkdir arm
    cd arm
    cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="${ARM_TOOLCHAIN_FILE}" -DMCPI_BUILD_MODE=arm -DMCPI_IS_MIXED_BUILD=ON -DMCPI_SERVER_MODE="${server_mode}" "$@" ../../..
    cd ../

    # Build Native Components
    mkdir native
    cd native
    cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" -DMCPI_BUILD_MODE=native -DMCPI_IS_MIXED_BUILD=ON -DMCPI_SERVER_MODE="${server_mode}" "$@" ../../..
    cd ../

    # Exit
    cd ../../
}

# Setup For ARM
armhf_setup() {
    # Create Build Dir
    rm -rf "build/${MODE}-armhf"
    mkdir -p "build/${MODE}-armhf"
    cd "build/${MODE}-armhf"

    # Prepare
    local server_mode='OFF'
    if [ "${MODE}" = "server" ]; then
        server_mode='ON'
    fi

    # Build All Components
    cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="${ARM_TOOLCHAIN_FILE}" -DMCPI_BUILD_MODE=both -DMCPI_SERVER_MODE="${server_mode}" "$@" ../..

    # Exit
    cd ../../
}

# Variables
MODE="$1"
ARCH="$2"
shift 2

# Verify Mode
if [ "${MODE}" != "client" ] && [ "${MODE}" != "server" ]; then
    echo "Invalid Mode: ${MODE}" > /dev/stderr
    exit 1
fi

# Setup
if [ "${ARCH}" = "armhf" ]; then
    armhf_setup "$@"
else
    setup "$@"
fi
