# Beginner Modding
The easiest way to create mods is by simply calling pre-existing functions.

## Utility Functions
MCPI-Reborn's built-in mods expose quite a few utility functions to making modding easier. These include:
* `misc_run_on_tick` will register a callback that will be run on every game-tick.
* `misc_run_on_init` will register a callback that will be run immediately after the game initializes.
* `screenshot_take` will take a screenshot.
* `compat_request_exit` will stop the game.
* More can be found [here](../../mods/include).

This project also includes a utility library: `libreborn`. This includes:
* `INFO`/`ERR` handle logging messages.
* `to_`/`from_cp437` will convert to/from MCPI's built-in text encoding.
* `home_get` will retrieve the profile directory.
* More can be found [here](../../libreborn/include).

## Minecraft Functions
You can also directly call many built-in game functions. For instance, `Gui::addMessage` will post a client-only message to the chat.

Unfortunately, not every function is supported. The list of supported functions/properties can be found [here](../../symbols/src).

If you really need a missing function, open a bug report.

## "Hooking" Functions
Functions from both MCPI-Reborn and the C library can be "hooked" using a similar technique to [`LD_PRELOAD`](https://tbrindus.ca/correct-ld-preload-hooking-libc/). To make this easier, `libreborn` provides the `HOOK` macro.

An example can be found [here](../../example-mods/chat-commands/src/chat-commands.cpp).

**Note:** This *does not* work with game or `static` functions.

## Extending Minecraft Classes
To create custom blocks, screens, or other components, you need to extend the corresponding class (for instance `Tile`).

However, due to how MCPI-Reborn is implemented, these classes cannot be directly extended. Instead, it provides `Custom*` wrapper classes that can be extended.

For instance, to create a custom block:
* Create a class extending `CustomTile`.
* Create a function that constructs the class.
* Register that function with `misc_run_on_tiles_setup`.
* A full example can be found [here](../../example-mods/custom-block/src/custom-block.cpp).

**Note:** A wrapper object *cannot* be directly cast to its corresponding plain object (or vice versa).

To retrieve the plain object from a wrapper, use the `self` property.

And to get the wrapper corresponding to a plain object, use the `custom_get<T>` function. However, this function *does not* check that the provided pointer is attached to a wrapper.