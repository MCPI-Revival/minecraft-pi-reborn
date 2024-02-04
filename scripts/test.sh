#!/bin/sh

set -e

# Variables
MODE="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
ARCH='host'

# Build
./scripts/build.mjs none "${MODE}" "${ARCH}" -DMCPI_HEADLESS_MODE=ON

# Add To PATH
export PATH="$(pwd)/out/${MODE}/${ARCH}/usr/bin:${PATH}"

# Make Test Directory
TEST_WORKING_DIR="$(pwd)/.testing-tmp"
rm -rf "${TEST_WORKING_DIR}"
mkdir -p "${TEST_WORKING_DIR}"

# Run
if [ "${MODE}" = "server" ]; then
    # Server Test
    cd "${TEST_WORKING_DIR}"
    minecraft-pi-reborn-server --only-generate
else
    # Client Test
    export _MCPI_SKIP_ROOT_CHECK=1
    export HOME="${TEST_WORKING_DIR}"
    minecraft-pi-reborn-client --default --no-cache --benchmark
fi

# Clean Up
rm -rf "${TEST_WORKING_DIR}"
