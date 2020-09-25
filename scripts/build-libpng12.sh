#!/bin/sh

set -e

git clone --depth 1 https://git.code.sf.net/p/libpng/code libpng -b libpng12

cd libpng

./configure --host arm-linux-gnueabihf --prefix /usr/arm-linux-gnueabihf

make -j$(nproc)
make install

cd ../
rm -rf libpng
