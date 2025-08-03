#!/bin/sh

set -e
cd "$(dirname "$0")"

# Arguments
IS_TAG="$1"
REF="$2"
echo "Releasing: ${REF}"

# Run Release
if [ "${IS_TAG}" = 'true' ]; then
    # Building Tag
    ./upload-packages.sh stable
    ./create.sh "${REF}"
elif [ "${IS_TAG}" = 'false' ]; then
    # Standard Commit
    ./upload-packages.sh unstable
else
    # Error
    exit 1
fi