#!/bin/sh

set -e

# Run
docker run --rm -v "$(pwd):/data" buildpack-deps:bullseye sh -c "cd /data; ./scripts/ci/run.sh"
