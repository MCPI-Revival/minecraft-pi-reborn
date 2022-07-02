#!/bin/sh

set -e

# Build
build() {
    # Use Build Dir
    if [ ! -f "build/${MODE}-${ARCH}/arm/build.ninja" ] || [ ! -f "build/${MODE}-${ARCH}/native/build.ninja" ]; then
        ./scripts/setup.sh "${MODE}" "${ARCH}"
    fi
    cd "build/${MODE}-${ARCH}"

    # Create Prefix
    local prefix="$(cd ../../; pwd)/out/${MODE}-${ARCH}"
    rm -rf "${prefix}"
    mkdir -p "${prefix}"

    # Build ARM Components
    cd arm
    cmake --build .
    DESTDIR="${prefix}" cmake --install .
    cd ../

    # Build Native Components
    cd native
    cmake --build .
    DESTDIR="${prefix}" cmake --install .
    cd ../

    # Exit
    cd ../../
}

# Variables
MODE="$1"
ARCH="$2"
shift 2

# Build
build "${MODE}" "${ARCH}"
