# `display-lists` Mod
This mod optimizes rendering using [OpenGL display lists](https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/glGenLists.xml).

It also enables offloading `Chunk::rebuild`
calls to another thread.

This applies to terrain and font rendering.
