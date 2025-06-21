#!/bin/sh

# Setup
set -e
cd "$(dirname "$0")"
. ./common.sh

# Arguments
DISTRIBUTION="$1"
COMPONENT='main'

# Delete Package
delete() {
    curl \
        -X DELETE \
        -H "Authorization: token ${RELEASE_TOKEN}" \
        "${SERVER}/api/packages/${ORGANIZATION}/debian/pool/${DISTRIBUTION}/${COMPONENT}/$1/$2/$3" \
        > /dev/null || :
}

# Upload Package
upload() {
    # Interpret Path
    FILE="$1"
    PACKAGE="$(dpkg-deb --field "${FILE}" Package)"
    VERSION="$(dpkg-deb --field "${FILE}" Version)"
    ARCH="$(dpkg-deb --field "${FILE}" Architecture)"

    # Delete Old Package
    delete "${PACKAGE}" "${VERSION}" "${ARCH}"

    # Upload
    curl \
        --upload-file "${FILE}" \
        -H "Authorization: token ${RELEASE_TOKEN}" \
        "${SERVER}/api/packages/${ORGANIZATION}/debian/pool/${DISTRIBUTION}/${COMPONENT}/upload" \
        > /dev/null
}

# Collect Packages
find ./out -type f -name '*.deb' | \
    while read -r file; do \
        upload "${file}"; \
    done