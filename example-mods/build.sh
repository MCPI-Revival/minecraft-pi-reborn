#!/bin/sh

set -e

# Create Output Directory
ROOT="$(pwd)"
OUT="${ROOT}/out"
rm -rf "${OUT}"
mkdir -p "${OUT}"

# Build
build() {
    cd "${ROOT}/$1"
    # Build
    rm -rf build
    mkdir build
    cd build
    cmake -GNinja ..
    cmake --build .
    # Copy Result
    cp lib*.so "${OUT}"
}
build chat-commands
build expanded-creative
build recipes
