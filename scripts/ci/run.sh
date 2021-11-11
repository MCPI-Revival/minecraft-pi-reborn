#!/bin/sh

set -e

# Install sudo
apt-get update
apt-get install -y sudo

# Prepare
export ARM_PACKAGES_SUPPORTED=1

# Install Dependencies
echo '==== Installing Dependencies ===='
./scripts/install-dependencies.sh

# Build
echo '==== Building & Packaging ===='
rm -rf out build
./scripts/package.sh amd64
./scripts/package.sh arm64
./scripts/package.sh armhf

# Test
echo '==== Testing ===='
./scripts/test.sh
