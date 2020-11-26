#!/bin/sh

set -e

git clone --depth 1 https://git.code.sf.net/p/libpng/code libpng -b libpng12

cd libpng

./configure --prefix /usr

make -j$(nproc)
make install

cd ../

rm -rf libpng
