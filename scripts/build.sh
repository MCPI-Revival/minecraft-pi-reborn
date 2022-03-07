#!/bin/sh

set -e

# Build
build() {
    # Use Build Dir
    if [ ! -d "build/${MODE}-${ARCH}" ]; then
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

# Build For ARM
armhf_build() {
    # Use Build Dir
    if [ ! -d "build/${MODE}-armhf" ]; then
        ./scripts/setup.sh "${MODE}" armhf
    fi
    cd "build/${MODE}-armhf"

    # Create Prefix
    local prefix="$(cd ../../; pwd)/out/${MODE}-armhf"
    rm -rf "${prefix}"
    mkdir -p "${prefix}"

    # Build All Components
    cmake --build .
    DESTDIR="${prefix}" cmake --install .

    # Exit
    cd ../../
}

# Variables
MODE="$1"
ARCH="$2"
shift 2

# Build
if [ "${ARCH}" = "armhf" ]; then
    armhf_build "${MODE}"
else
    build "${MODE}" "${ARCH}"
fi
