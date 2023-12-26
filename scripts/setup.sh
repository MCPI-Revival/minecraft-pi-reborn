#!/bin/sh

set -e

# ARM Toolchain File
ARM_TOOLCHAIN_FILE="$(pwd)/cmake/toolchain/armhf-toolchain.cmake"

# Variables
MODE="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
ARCH="$(echo "$2" | tr '[:upper:]' '[:lower:]')"
shift 2

# Verify Mode
if [ "${MODE}" != "client" ] && [ "${MODE}" != "server" ]; then
    echo "Invalid Mode: ${MODE}" > /dev/stderr
    exit 1
fi

# Find Toolchain
toolchain_file="$(pwd)/cmake/toolchain/${ARCH}-toolchain.cmake"
if [ ! -f "${toolchain_file}" ]; then
    echo "Invalid Architecture: ${ARCH}" > /dev/stderr
    exit 1
fi

# Create Build Dir
rm -rf "build/${MODE}-${ARCH}"
mkdir -p "build/${MODE}-${ARCH}"
cd "build/${MODE}-${ARCH}"

# Server Build
server_mode='OFF'
if [ "${MODE}" = "server" ]; then
    server_mode='ON'
fi
# Mixed Build
mixed_build='ON'
if [ "${ARCH}" = "armhf" ]; then
    mixed_build='OFF'
fi
# Extra Flags
extra_flags="-DMCPI_IS_MIXED_BUILD=${mixed_build} -DMCPI_SERVER_MODE=${server_mode}"

# Build ARM Components
mkdir arm
cd arm
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE="${ARM_TOOLCHAIN_FILE}" -DMCPI_BUILD_MODE=arm ${extra_flags} "$@" ../../..
cd ../

# Build Native Components
mkdir native
cd native
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" -DMCPI_BUILD_MODE=native ${extra_flags} "$@" ../../..
cd ../

# Exit
cd ../../
