# Example Mod
This is an example of a mod that cane be built using the modding SDK.

This specific mod adds even more items and blocks to the Creative Inventory. It was originally by [@Bigjango13](https://github.com/bigjango13).

## The SDK
The modding SDK is a collection of exported CMake targets that allows anyone to create their own MCPI mod!

The SDK is copied to ``~/.minecraft-pi/sdk/lib/minecraft-pi-reborn-client/sdk/sdk.cmake`` whenever MCPI-Reborn is started.

## How do I use this?
```sh
mkdir build
cd build
cmake ..
cp libexpanded-creative.so ~/.minecraft-pi/mods
```
