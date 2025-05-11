---
gitea: none
include_toc: true
---

# Intermediate Modding
This chapter will cover intercepting or altering the behavior of class methods.

This chapter assumes basic knowledge of C++.

## Example Target Structures
This guide will use the following structures to demonstrate the modding API:
```c++
struct A {
    virtual bool func(int x, int y);
};
struct B : A {
};
```

> [!IMPORTANT]
> In reality, these functions only support a subset of game functions.

## Method Objects
Targetable methods have corresponding objects that can be used in the following functions.
For instance, `A::func` would correspond to the `A_func` object.

They also have the following additional members:
* `name`: The method's name (including its enclosing structure).
* `backup`: A pointer to the original method.
* `get`: A method that will retrieve a pointer to the method.
  * It accepts a single boolean argument. This should bet to `true` if (and only if) the returned value will be stored.
* `get_vtable_addr`: A method that will return a pointer to the method's [VTable](https://en.wikipedia.org/wiki/Virtual_method_table?useskin=vector) entry (if applicable).
* Any additional members are implementation details and *should not be relied upon*.

## `overwrite_calls`
This function globally intercepts all calls to a function.
This includes calls originating from other mods.
This supports both virtual and non-virtual functions.

This allows layered injections, where each function can call the previous version.

The naming convention for replacement functions is `<target function>_injection`.

### Example
```c++
// The Replacement Function
static bool A_func_injection(A_func_t original, A *self, int a, int b) {
    INFO("Hello World!");
    // Call Original Method
    return original(self, a, b);
}
// Inside Init Function
overwrite_calls(A_func, A_func_injection);
```

### Limitations
This function can only target methods that actually exist.
For instance, `B::func` is not a real method as `B` just inherits it without overriding it. It does not have any actual code to target.

## `patch_vtable`
This function directly patches a method's VTable entry. This can be used to work around the previous method's limitations.

### Example
```c++
// The Replacement Function
static bool B_func_injection(B *self, int a, int b) {
    INFO("Hello World!");
    // Call Original Method
    return A_func->get(false)((A *) self, a, b);
}
// Inside Init Function
patch_vtable(B_func, B_func_injection);
```

### Limitations
This function does not support automatic layering like the previous method.
This means you have to manually retrieve the correct parent function to call.
It may also cause conflicts with other mods.

This method also does not affect subclasses which inherit or override the target.

It is highly recommended that `overwrite_calls` should be used whenever possible.