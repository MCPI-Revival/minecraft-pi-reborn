#!/bin/sh

set -e

# Build Test
ARCH="$(dpkg-architecture -qDEB_BUILD_ARCH)"
./scripts/setup.sh server "${ARCH}"
./scripts/build.sh server "${ARCH}"

# Add minecraft-pi-reborn-legacy-server To PATH
export PATH="$(pwd)/out/server-$(dpkg-architecture -qDEB_BUILD_ARCH)/usr/bin:${PATH}"

# Create Test Directory
rm -rf build/test
mkdir -p build/test

# Run Test
cd build/test
minecraft-pi-reborn-legacy-server --only-generate
cd ../../

# Build Benchmark
./scripts/setup.sh client "${ARCH}" -DMCPI_HEADLESS_MODE=ON
./scripts/build.sh client "${ARCH}"

# Add minecraft-pi-reborn-legacy-client To PATH
export PATH="$(pwd)/out/client-$(dpkg-architecture -qDEB_BUILD_ARCH)/usr/bin:${PATH}"

# Skip Root Check
export _MCPI_SKIP_ROOT_CHECK=1

# Run Benchmark
export HOME="$(pwd)/build/test"
minecraft-pi-reborn-legacy-client --default --no-cache --benchmark
