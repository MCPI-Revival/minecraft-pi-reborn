#!/bin/sh

# Current Version
DEB_VERSION='1.0.0'

set -e

# Docker Messes With SetGID
chmod -R g-s debian

# Clean out Directory
rm -rf out
mkdir -p out/deb

# Prepare
rm -rf debian/tmp
mkdir debian/tmp

# Set Version
prepare_version() {
    sed -i 's/${VERSION}/'"${DEB_VERSION}.$(date --utc '+%Y%m%d.%H%M')"'/g' "$1/DEBIAN/control"
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
    prepare_version "debian/tmp/$1"
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
    prepare_version debian/tmp/server
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
