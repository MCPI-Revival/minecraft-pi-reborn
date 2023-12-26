#!/bin/sh

set -e

# Variables
MODE="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
ARCH="$(echo "$2" | tr '[:upper:]' '[:lower:]')"

# Run CMake If Needed
if [ ! -f "build/${MODE}-${ARCH}/arm/build.ninja" ] || [ ! -f "build/${MODE}-${ARCH}/native/build.ninja" ]; then
    ./scripts/setup.sh "${MODE}" "${ARCH}"
fi
# Use Build Dir
cd "build/${MODE}-${ARCH}"

# Create Prefix
if [ -z "${DESTDIR+x}" ]; then
    export DESTDIR="$(cd ../../; pwd)/out/${MODE}-${ARCH}"
    rm -rf "${DESTDIR}"
    mkdir -p "${DESTDIR}"
fi

# Build ARM Components
cd arm
cmake --build .
cmake --install .
cd ../

# Build Native Components
cd native
cmake --build .
cmake --install .
cd ../

# Exit
cd ../../
