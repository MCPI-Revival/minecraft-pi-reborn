#!/bin/sh

set -e

# Create Output Directory
ROOT="$(pwd)"
OUT="${ROOT}/out/example-mods"
rm -rf "${OUT}"
mkdir -p "${OUT}"

# Build
for MOD in example-mods/*/; do
    cd "${ROOT}/${MOD}"
    # Build
    rm -rf build
    mkdir build
    cd build
    cmake -GNinja ..
    cmake --build .
    # Copy Result
    cp lib*.so "${OUT}"
done
