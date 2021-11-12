#!/bin/sh

set -e

# Build
build() {
    # Find Toolchain
    local toolchain_file="$(pwd)/cmake/$2-toolchain.cmake"
    if [ ! -f "${toolchain_file}" ]; then
        echo "Invalid Architecture: $2" > /dev/stderr
        exit 1
    fi

    # Create Build Dir
    rm -rf "build/$1-$2"
    mkdir -p "build/$1-$2"
    cd "build/$1-$2"

    # Create Prefix
    local prefix="$(cd ../../; pwd)/out/$1-$2"
    rm -rf "${prefix}"
    mkdir -p "${prefix}"

    # Prepare
    local extra_arg='-DMCPI_USE_MEDIA_LAYER_PROXY=ON'
    if [ "$1" = "server" ]; then
        extra_arg='-DMCPI_SERVER_MODE=ON'
    fi

    # Build ARM Components
    mkdir arm
    cd arm
    cmake -DMCPI_BUILD_MODE=arm "${extra_arg}" ../../..
    make -j$(nproc)
    make install DESTDIR="${prefix}"
    cd ../

    # Build Native Components
    mkdir native
    cd native
    cmake -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" -DMCPI_BUILD_MODE=native "${extra_arg}" ../../..
    make -j$(nproc)
    make install DESTDIR="${prefix}"
    cd ../

    # Exit
    cd ../../
}

# Build For ARM
armhf_build() {
    # Create Build Dir
    rm -rf "build/$1-armhf"
    mkdir -p "build/$1-armhf"
    cd "build/$1-armhf"

    # Create Prefix
    local prefix="$(cd ../../; pwd)/out/$1-armhf"
    rm -rf "${prefix}"
    mkdir -p "${prefix}"

    # Prepare
    local server_mode='OFF'
    if [ "$1" = "server" ]; then
        server_mode='ON'
    fi

    # Build All Components
    cmake -DMCPI_BUILD_MODE=both -DMCPI_SERVER_MODE="${server_mode}" ../..
    make -j$(nproc)
    make install DESTDIR="${prefix}"

    # Exit
    cd ../../
}

# Verify Mode
if [ "$1" != "client" ] && [ "$1" != "server" ]; then
    echo "Invalid Mode: $1" > /dev/stderr
    exit 1
fi

# Build
if [ "$2" = "armhf" ]; then
    armhf_build "$1"
else
    build "$1" "$2"
fi
