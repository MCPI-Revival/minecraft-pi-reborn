---
gitea: none
include_toc: true
---

# Beginner Modding
This chapter will cover basic modding techniques.
This includes MCPI-Reborn's built-in API and game functions/classes.

This chapter assumes basic knowledge of C++.

## Utility Functions
MCPI-Reborn's built-in mods expose quite a few utility functions to make modding easier. These include:

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

## Minecraft Classes
The game contains many built-in classes.
Accessing, manipulating, creating, and destroying these classes will be essential for effective modding.

Unfortunately, not every class is supported. The list of supported classes can be found [here](../../symbols/src).

### "Simple" Classes
Some built-in classes have been classified as "simple."

This means they have been fully defined, have no inheritance, and have no virtual functions.

These are typically "data classes" like `ItemInstance`.

These classes can be treated just like any other C++ `struct`.
They can also be easily copied and allocated on the stack, something explicitly blocked with other game classes.

### Member Functions And Properties
All classes have member functions and/or properties. These can be called, accessed, and modified normally.

For instance:

* Functions
  * `Gui::addMessage`
  * `Level::setTile`
  * `Item::initItems` (Static)
* Properties
  * `Minecraft::level`
  * `Tile::id`
  * `Gui::GuiScale` (Static)

Unfortunately, most classes have not been fully mapped. This means that many properties/functions are not accessible.
If you really need a missing member, open a bug report.

### Extending Classes
Due to how MCPI-Reborn is implemented, game classes cannot be directly extended.
Instead, `Custom*` wrapper classes are provided.

For instance, to create a custom block:

* Create a class extending `CustomTile`.
* Create a function that constructs the class.
* Register that function with `misc_run_on_tiles_setup`.
* A full example can be found [here](../../example-mods/custom-block/src/custom-block.cpp).

> [!WARNING]
> A wrapper object *cannot* be directly cast to its corresponding plain object (or vice versa).

To retrieve the plain object from a wrapper, use the `self` property.

And to get the wrapper corresponding to a plain object, use the `custom_get<T>` function.
However, this function *does not* check that the provided pointer is attached to a wrapper.
Make sure you know exactly what object you're working with before using this.

### Construction
Due to technical limitations, game classes cannot be directly constructed using `new`.
Instead, the class must be manually allocated before calling the constructor separately.

You must *always* call a constructor.

For instance:
```c++
Textures *textures = Textures::allocate();
textures->constructor(&minecraft->options, minecraft->platform());
```

Or:
```c++
ProgressScreen *screen = ProgressScreen::allocate();
screen->constructor();
```

> [!TIP]
> If a constructor does not exist (usually because of inlining), you must implement it manually.
>
> This usually involves:
>
> 1. Calling the parent constructor.
> 2. Setting the correct [VTable](https://en.wikipedia.org/wiki/Virtual_method_table?useskin=vector).
> 3. Constructing all properties (using placement `new`).
>
> For instance:
> ```c++
> SendInventoryPacket *packet = SendInventoryPacket::allocate();
> ((Packet *) packet)->constructor();
> packet->vtable = SendInventoryPacket::VTable::base;
> new (&packet->items) std::vector<ItemInstance>;
> ```

### Destruction
Game classes cannot be directly destroyed using `delete`.
Instead, the destructor must be called manually.

For instance:
```c++
Screen *screen = get_screen();
screen->destructor_deleting();
```

## "Hooking" Functions
Some functions with can be "hooked" using a similar technique to [`LD_PRELOAD`](https://tbrindus.ca/correct-ld-preload-hooking-libc/).
To make this easier, `libreborn` provides the `HOOK` macro.

This only applies to function with [external-linkage](https://learn.microsoft.com/en-us/cpp/cpp/program-and-linkage-cpp?view=msvc-170#external-vs-internal-linkage). For instance:

* Supported
  * `open`
  * `close`
  * `chat_send_message_to_clients`
* Unsupported
  * `Gui::addMessage`
  * `Minecraft::tick`
  * `main`

The signature is `HOOK(<name>, <return type>, <args>) { <code> }`.
It will also generate a `real_<name>` method that will return the real/original/next version of the target.

> [!NOTE]
> If the target function does not use [C linkage](https://en.cppreference.com/w/cpp/language/language_linkage),
> then you must pass the mangled name to the macro.

A full example can be found [here](../../example-mods/chat-commands/src/chat-commands.cpp).
