#!/bin/sh

set -e

# Run
act push -W '.gitea/workflows/build.yml'
