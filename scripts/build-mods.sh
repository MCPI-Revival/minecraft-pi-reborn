#!/bin/sh

set -e

cd mods

mkdir build
cd build

cmake -DCMAKE_TOOLCHAIN_FILE=../../cmake/toolchain.cmake ..
make -j$(nproc)

cd ../../

mkdir minecraft-pi/mods
cp mods/build/lib*.so minecraft-pi/mods

cp mods/build/core/lib*.so minecraft-pi
cp mods/build/core/launcher minecraft-pi
