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
echo '==== Building ===='
./scripts/build-all.sh

# Test
echo '==== Testing ===='
./scripts/test.sh

# Package
echo '==== Packaging ===='
./scripts/package.sh
