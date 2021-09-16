#!/bin/sh

set -e

# This Script Assumes An x86_64 Host
if [ "$(uname -m)" != "x86_64" ]; then
    echo 'Invalid Host Architecture' > /dev/stderr
    exit 1
fi

# Build For x86_64
native_build() {
    # Create Build Dir
    rm -rf build/$1-x86_64
    mkdir -p build/$1-x86_64
    cd build/$1-x86_64

    # Create Prefix
    local prefix="$(cd ../../; pwd)/out/$1-x86_64"
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
    cmake -DMCPI_BUILD_MODE=native "${extra_arg}" ../../..
    make -j$(nproc)
    make install DESTDIR="${prefix}"
    cd ../

    # Exit
    cd ../../
}

# Build For ARM64
arm64_build() {
    # Create Build Dir
    rm -rf build/$1-arm64
    mkdir -p build/$1-arm64
    cd build/$1-arm64

    # Create Prefix
    local prefix="$(cd ../../; pwd)/out/$1-arm64"
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
    cmake -DCMAKE_TOOLCHAIN_FILE="$(cd ../../../; pwd)/cmake/arm64-toolchain.cmake" -DMCPI_BUILD_MODE=native "${extra_arg}" ../../..
    make -j$(nproc)
    make install DESTDIR="${prefix}"
    cd ../

    # Exit
    cd ../../
}

# Build For ARM
arm_build() {
    # Create Build Dir
    rm -rf build/$1-arm
    mkdir -p build/$1-arm
    cd build/$1-arm

    # Create Prefix
    local prefix="$(cd ../../; pwd)/out/$1-arm"
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
if [ "$2" != "client" ] && [ "$2" != "server" ]; then
    echo "Invalid Mode: $2" > /dev/stderr
    exit 1
fi

# Build
if [ "$1" = "native" ]; then
    native_build "$2"
elif [ "$1" = "arm64" ]; then
    arm64_build "$2"
elif [ "$1" = "arm" ]; then
    arm_build "$2"
else
    echo "Invalid Architecture: $1" > /dev/stderr
    exit 1
fi
