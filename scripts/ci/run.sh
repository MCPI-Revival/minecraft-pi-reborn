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

# Build/Package
echo '==== Building & Packaging ===='
./scripts/package-all.sh

# Test
echo '==== Testing ===='
./scripts/test.sh
