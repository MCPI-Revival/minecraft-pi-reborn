#!/bin/sh

set -e

# Docker Messes With SetGID
chmod -R g-s debian

# Allow minecraft-pi Script To Use Docker
chmod u+s debian/usr/bin/minecraft-pi

# Clean out Directory
rm -rf out
mkdir out

# Generate DEB
dpkg -b debian out
