#!/bin/sh

set -e

# Docker Messes With SetGID
chmod -R g-s debian

# Clean out Directory
rm -rf out
mkdir out

# Generate DEB
dpkg -b debian out
