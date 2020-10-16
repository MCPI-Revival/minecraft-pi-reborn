#!/bin/sh

set -e

# Docker Messes With SetGID
chmod -R g-s debian

# Clean out Directory
rm -rf out
mkdir -p out/deb

# Package Client DEBs
package_client() {
    rm -rf debian/tmp
    rsync -r debian/client/common/ debian/tmp
    rsync -r "debian/client/$1/" debian/tmp
    dpkg -b debian/tmp out/deb
    rm -rf debian/tmp
}
package_client virgl
package_client native

# Package Server DEB
dpkg -b debian/server out/deb

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
