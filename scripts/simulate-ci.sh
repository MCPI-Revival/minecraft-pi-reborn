#!/bin/sh

set -e

# Change Directory
cd "$(dirname "$0")/../"

# Run
act push -W '.gitea/workflows/build.yml'
