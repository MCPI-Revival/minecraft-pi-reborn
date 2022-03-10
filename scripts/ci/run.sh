#!/bin/sh

set -e

# Build/Package
echo '==== Building & Packaging ===='
./scripts/package-all.sh

# Test
echo '==== Testing ===='
./scripts/test.sh
