#!/bin/sh

set -e

# Prepare
rm -f debian/changelog
PACKAGE='minecraft-pi-reborn'
VERSION="$(cat VERSION)"
DISTRO="$(lsb_release -cs)"
EDITOR='true' NAME='TheBrokenRail' EMAIL='connor24nolan@live.com' dch -u low -v "${VERSION}" --create --distribution "${DISTRO}" --package "${PACKAGE}" "Release ${VERSION}"

# Custom Architecture
ARCH="$(dpkg-architecture -qDEB_BUILD_ARCH)"
if [ -z "$1" ]; then
    ARCH="$1"
fi

# Build
export DEB_CUSTOM_OUTPUT_DIR='out'
debuild --no-lintian -a"${ARCH}" -us -uc --buildinfo-option=-u"${DEB_CUSTOM_OUTPUT_DIR}" --changes-option=-u"${DEB_CUSTOM_OUTPUT_DIR}" -b
