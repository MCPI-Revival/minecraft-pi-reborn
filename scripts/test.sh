#!/bin/sh

set -e

# Add minecraft-pi-reborn-server To PATH
export PATH="$(pwd)/out/server-x86_64/usr/bin:${PATH}"

# Create Test Directory
rm -rf build/test
mkdir -p build/test

# Run Test
cd build/test
minecraft-pi-reborn-server --only-generate
