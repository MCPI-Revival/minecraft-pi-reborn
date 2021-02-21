#!/bin/sh

set -e

git clone --depth 1 https://git.code.sf.net/p/libpng/code libpng -b libpng12

cd libpng

./configure

make -j$(nproc)

mkdir -p ../minecraft-pi/lib
cp -L .libs/libpng12.so.0 ../minecraft-pi/lib

cd ../

rm -rf libpng
