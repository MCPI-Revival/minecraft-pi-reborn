#!/bin/sh

set -e

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

# Build Components
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" -DMCPI_SERVER_MODE="${server_mode}" "$@" ../../

# Exit
cd ../../
