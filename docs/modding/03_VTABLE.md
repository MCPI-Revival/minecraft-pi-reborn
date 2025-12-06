---
gitea: none
include_toc: true
---

# Introduction To VTables
One of C++'s most prominent features is [virtual functions](https://en.cppreference.com/w/cpp/language/virtual.html).
This is a special type of function that can be overridden or extended by subclasses.

For instance, a subclass can override `Tile::onPlace`.
This would allow blocks to implement custom behavior when placed in the world.

To understand how this is implemented, one must first understand how C++ implements normal/non-virtual functions.

This chapter assumes basic knowledge of C++.

## Normal Functions
Non-virtual member functions are an illusion in C++.
While they may appear special, in reality they are no different from standard C functions.

For instance, calling `gui->tick()` (where `gui` is of type `Gui *`) is really just a call to `_ZN3Gui4tickEv(gui)`.

This is great because it means member functions have no performance penalty.
But it relies on the compiler always knowing exactly which function implementation corresponds to a given function call,
which is almost never true for virtual functions.

This is because `Tile::onPlace` might be `_ZN4Tile7onPlaceEP5Leveliii`, `_ZN9HeavyTile7onPlaceEP5Leveliii`,
`_ZN9TorchTile7onPlaceEP5Leveliii`, or maybe even a function that has not been defined yet.

To solve this, C++ uses a technique called VTables.

## VTables
A [VTable](https://en.wikipedia.org/wiki/Virtual_method_table?useskin=vector)
(also known as a VFTable or a virtual function table)
is a hidden property that exists in every class that has virtual functions.

It is a table of function pointers that point to the correct implementation of each virtual function.

Every virtual function has an index in this table.
To call a virtual function, access and call the function pointer at the given index.
For instance, to call `Tile::onPlace`:

1. Access the hidden VTable property (usually located at offset `0x0`).
2. Retrieve the function pointer at index `0x68` (the index of `Tile::onPlace`).
3. Call the function pointer.

> [!TIP]
> MCPI-Reborn simplifies accessing the VTable.
> For instance, you can call `tile->vtable->onPlace(tile, ...)` or even `tile->onPlace(...)`.
> It also publicly defines the VTable for every class.
> For instance, `Tile`'s VTable property is of type `const Tile::VTable *`
> and a global reference is available at `Tile::VTable::base`.
> An object's `std::type_info` can be accessed using `obj->vtable->get_full()->info`.

## Relevance To Modding
VTables are extremely useful when modding C++.
This is because they are straightforward to modify.

By modifying a VTable, a mod can replace any virtual function in any class.

MCPI-Reborn provides multiple functions that can be used to modify VTables. They will be covered in the next chapter.

VTables can also be used to check a given object's type.
For instance, `tile->vtable == (const Tile::VTable *) HeavyTile::VTable::base`
will return `true` if `tile` is a `HeavyTile`.
Note: this does not consider subclasses. This will return `false` if `tile` is a subclass of `HeavyTile`.
