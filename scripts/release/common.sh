#!/bin/sh

# Enter Repository Root
cd ../../

# Constants
SERVER='https://gitea.thebrokenrail.com'
ORGANIZATION='minecraft-pi-reborn'
REPOSITORY="${ORGANIZATION}"
SLUG="${ORGANIZATION}/${REPOSITORY}"
echo "Target: ${SERVER}/${SLUG}"
VERSION="$(cat VERSION)"
echo "Version: ${VERSION}"