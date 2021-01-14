#!/bin/sh

# Current Version
DEB_VERSION='1.0.0'

# Dependencies
REQUIRED_DOCKER_VERSION='19.03'
COMMON_DEPENDENCIES="docker.io (>=${REQUIRED_DOCKER_VERSION}) | docker-ce (>=${REQUIRED_DOCKER_VERSION}), libseccomp2 (>=2.4.2), docker-compose, binfmt-support"
CLIENT_DEPENDENCIES="zenity, policykit-1, passwd, login, x11-xserver-utils"
RECOMMENDED_DEPENDENCIES="qemu-user-static"

set -e

# Docker Messes With SetGID
chmod -R g-s debian

# Clean out Directory
rm -rf out
mkdir -p out/deb

# Prepare
rm -rf debian/tmp
mkdir debian/tmp

# Prepare DEBIAN/control
prepare_control() {
    sed -i 's/${VERSION}/'"${DEB_VERSION}.$(date --utc '+%Y%m%d.%H%M')"'/g' "$1/DEBIAN/control"
    sed -i 's/${DEPENDENCIES}/'"${COMMON_DEPENDENCIES}$2"'/g' "$1/DEBIAN/control"
    sed -i 's/${RECOMMENDED_DEPENDENCIES}/'"${RECOMMENDED_DEPENDENCIES}$2"'/g' "$1/DEBIAN/control"
}

# Package Client DEBs
docker save thebrokenrail/minecraft-pi:client | gzip > debian/tmp/client-image.tar.gz
package_client() {
    # Clean
    rm -rf "debian/tmp/$1"
    # Prepare
    rsync -r debian/client/common/ "debian/tmp/$1"
    rsync -r "debian/client/$1/" "debian/tmp/$1"
    cp debian/tmp/client-image.tar.gz "debian/tmp/$1/usr/share/minecraft-pi/client/image.tar.gz"
    prepare_control "debian/tmp/$1" ", ${CLIENT_DEPENDENCIES}"
    # Build
    dpkg -b "debian/tmp/$1" out/deb
}
package_client virgl
package_client native

# Package Server DEB
docker save thebrokenrail/minecraft-pi:server | gzip > debian/tmp/server-image.tar.gz
package_server() {
    # Clean
    rm -rf debian/tmp/server
    # Prepare
    rsync -r debian/server/ debian/tmp/server
    cp debian/tmp/server-image.tar.gz debian/tmp/server/usr/share/minecraft-pi/server/image.tar.gz
    prepare_control debian/tmp/server ''
    # Build
    dpkg -b debian/tmp/server out/deb
}
package_server

# Clean Up
rm -rf debian/tmp

# Export Libraries
mkdir -p out/lib

# Copy Headers
cp -r mods/include out/lib/include

# Copy Shared Library
IMG_ID="$(docker create thebrokenrail/minecraft-pi:client)"
docker cp "${IMG_ID}":/app/minecraft-pi/mods/. ./out/lib/. || :
RET="$?"
docker rm -v "${IMG_ID}"
exit "${RET}"
