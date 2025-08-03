---
gitea: none
include_toc: true
---

# Introduction To Ghidra
[Ghidra](https://ghidra-sre.org/) is a tool for reverse-engineering binaries. It is extremely useful for game modding.

This is not intended to be a general guide, rather this chapter will focus on how Ghidra can help with modding Minecraft.

This chapter assumes basic knowledge of ARM32 Assembly.

## Common Actions
Ghidra contains *many* tools. These are some of the most useful for game modding:
* You can jump to a specific function or address using `Navigate -> Go To...`.
  * This tool supports wildcard expressions.
  * You can also jump to a specific string by entering `*<fragment>*`. This is because strings are automatically labeled as `s_<string>_<address>` (for instance `s_This_works?_00104a1f`).
* You can find the uses of a specific function or object by right-clicking it and selecting `Refernces -> Show Refernces to <Function>`.
  * This does not work with virtual methods.
* You can patch a specific instruction by right-clicking it and selecting `Patch Instruction`.
  * A helpful instruction is `nop`, this makes an instruction do absolutely nothing.

## Reverse-Engineering Functions
Ghidra can decompile game functions into pseudocode. This makes figuring out various game functions extremely easy.

One of the best ways to do this is to decompile the equivalent function in MCPE v0.6.1.
This is because this version is almost identical to MCPI while also including function names.

> [!WARNING]
> Property offsets are often (but not always) different between MCPE and MCPI.

For instance, the decompiled version of `ServerSideNetworkHandler::getPlayer` in MCPI is:
```c++
undefined4 FUN_00075464(int param_1,int *param_2)
{
  bool bVar1;
  int iVar2;
  uint uVar3;

  uVar3 = 0;
  while( true ) {
    iVar2 = *(int *)(*(int *)(param_1 + 0xc) + 0x60);
    if ((uint)(*(int *)(*(int *)(param_1 + 0xc) + 100) - iVar2 >> 2) <= uVar3) {
      return 0;
    }
    bVar1 = FUN_000d663c(param_2,(int *)(*(int *)(iVar2 + uVar3 * 4) + 0xc08));
    if (bVar1) break;
    uVar3 = uVar3 + 1;
  }
  return *(undefined4 *)(*(int *)(*(int *)(param_1 + 0xc) + 0x60) + uVar3 * 4);
}
```

Meanwhile, the decompiled version of the same function in MCPE is:
```c++
/* ServerSideNetworkHandler::getPlayer(RakNet::RakNetGUID const&) */
undefined4 __thiscall
ServerSideNetworkHandler::getPlayer(ServerSideNetworkHandler *this,RakNetGUID *param_1)
{
  int iVar1;
  uint uVar2;

  uVar2 = 0;
  while( true ) {
    iVar1 = *(int *)(*(int *)(this + 0xc) + 0x60);
    if ((uint)(*(int *)(*(int *)(this + 0xc) + 100) - iVar1 >> 2) <= uVar2) {
      return 0;
    }
    iVar1 = RakNet::RakNetGUID::operator==
                      (param_1,(RakNetGUID *)(*(int *)(iVar1 + uVar2 * 4) + 0xc48));
    if (iVar1 != 0) break;
    uVar2 = uVar2 + 1;
  }
  return *(undefined4 *)(*(int *)(*(int *)(this + 0xc) + 0x60) + uVar2 * 4);
}
```

## Finding An MCPI Function
Often, you need to find the address of an MCPI function from an already-known MCPE function.

The easiest way is to check if it is already documented.
This project contains a [list of known symbols](../../symbols/src) including function addresses.

If it is not documented, there are a few techniques that can be used to locate the function:
* If a function contains a unique string, you can search for this string inside MCPI using `Go To...`.
* If this function is called by or calls a known function, you can access that function inside MCPI and look through its callers and callees.
  * You may need to do this for multiple levels. For instance, an unknown function may be called by another unknown function, but that function may be called by a known function.
* If this function is virtual, you can find the equivalent VTable inside MCPI because MCPE and MCPI VTable indices are almost always the same.

> [!NOTE]
> Because of inlining, an MCPE function may not exist inside MCPI and vice versa.
> For instance, `CompoundTag::getCompound` is inlined in MCPI.
