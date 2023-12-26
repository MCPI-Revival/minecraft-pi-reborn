#!/bin/sh

set -e

# Variables
MODE="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
ARCH="$(dpkg-architecture -qDEB_BUILD_ARCH)"

# Build
./scripts/setup.sh "${MODE}" "${ARCH}" -DMCPI_HEADLESS_MODE=ON
./scripts/build.sh "${MODE}" "${ARCH}"

# Add To PATH
export PATH="$(pwd)/out/${MODE}-${ARCH}/usr/bin:${PATH}"

# Make Test Directory
rm -rf build/test
mkdir -p build/test

# Run
if [ "${MODE}" = "server" ]; then
    # Server Test
    cd build/test
    minecraft-pi-reborn-server --only-generate
    cd ../../
else
    # Client Test
    export _MCPI_SKIP_ROOT_CHECK=1
    export HOME="$(pwd)/build/test"
    minecraft-pi-reborn-client --default --no-cache --benchmark

    # Build Example Mods
    for project in example-mods/*/; do
        cd "${project}"
        rm -rf build
        mkdir build
        cd build
        cmake -GNinja ..
        cmake --build .
        cd ../../../
    done
fi
