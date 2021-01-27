#!/bin/sh

set -e

cd launcher

mkdir build
cd build

cmake ..
make -j$(nproc)
make install DESTDIR=../../minecraft-pi

cd ../../

cd mods

mkdir build
cd build

cmake ..
make -j$(nproc)
make install DESTDIR=../../minecraft-pi

cd ../../