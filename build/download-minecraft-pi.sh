#!/bin/sh

set -e

URL="https://www.minecraft.net/content/dam/minecraft/edition-pi/minecraft-pi-0.1.1.tar.gz"

mkdir minecraft-pi
wget -O - "${URL}" | tar -xz --strip-components 1 -C minecraft-pi
