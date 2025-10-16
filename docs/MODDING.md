# Modding

MCPI-Reborn has modding abilities that allows for users to make mods
that directly change MCPI or Reborn.

The mods are compiled into ARM shared libraries that are loaded from
`~/.minecraft-pi/mods` when MCPI launches.

Reborn loads mods when it launches, so MCPI will have to be relaunched
each time a mod is changed for the change to take effect.

C and C++ are the most common languages to mod Reborn with, while mods
might be possible in any programming languages that allow for
compilation into shared libraries, it is highly discouraged.

## Changing MCPI

The way that mods change MCPI is exactly how Reborn changes MCPI; by
patching and overwriting memory addresses.

To find an address a reverse engineering tool can be used (the most
popular one in the Revival community is the NSA's open source Ghidra
tool).

Another way is to use already known addresses such as the ones in
`minecraft.h` which is a header file that is full of useful addresses,
property offsets and functions that are very useful for modding.

Property offsets are another way to get or change data in MCPI, once you
have an object you can add or subtract a property offset to get a
different object from it.

For example if you had a `Player *` object and you wanted to get
an `Inventory *` object from it you would do:

```c
unsigned char *inventory = *(unsigned char **) (player + player_inventory_property_offset);
```

## Changing Reborn

Changing Reborn mostly uses a small but powerful macro called ***HOOK***
which allows mods to override or proxy calls to a function.

This cannot be done to static or internally visible functions.

## Example Mods

Here are two very basic example mods, they can be compiled with this
command: `arm-linux-gnueabihf-gcc -shared -fPIC -Wall -Wextra -O6 -o
libfile.so file.c`

### Hello log!

This mod will print a message to the terminal.

`hello-log.c`:

```c
// Needed for fprintf
#include <stdio.h>

// This is Reborns version of `int main`
__attribute__((constructor)) static void init() {
    // You will have to print to stderr because of the way Reborn works
    fprintf(stderr, "%s", "Hello log!\n");
}
```

### Goodbye log!

This will print a message when MCPI is closed

`goodbye-log.c`:

```c
// Needed for fprintf
#include <stdio.h>

// This is a destructor, it is called when MCPI is exiting
__attribute__((destructor)) static void cleanup() {
    // You will have to print to stderr because of the way Reborn works
    fprintf(stderr, "%s", "Goodbye log!\n");
}
```
