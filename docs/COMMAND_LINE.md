# Command Line Usage

## Command Line Arguments

### ``--version`` (Or ``-v``)
If you run MCPI-Reborn with ``--version`` it will print its version to ``stdout``.

### ``--debug``
This sets ``MCPI_DEBUG``.

### Client Mode Only

#### ``--print-available-feature-flags``
This print the available feature flags (and their default values) to ``stdout`` and then immediately exit.

The feature flags are printed in the following format:
```
TRUE This Flag Is On By Default
FALSE This Flag Is Off By Default
```

#### ``--default``
This will skip the startup configuration dialogs and just use the default values. This will use the cached configuration unless ``--no-cache`` is used.

#### ``--benchmark``
This will make MCPI-Reborn enter a simple benchmark mode. This means automatically loading a newly generated world, then rotating the camera for a period of time. When it has finished, it will then exit and print the average FPS while the world was loaded. In this mode, all user input is blocked. However you can still modify rendering settings by changing feature flags.

The world used will always be re-created on start and uses a hard-coded seed.

#### ``--no-cache``
This will skip loading and saving the cached launcher configuration.

#### ``--wipe-cache``
This will wipe the cached launcher configuration.

### Server Mode Only

#### ``--only-generate``
This will make MCPI-Reborn immediately exit once world generation has completed. This is mainly used for automatically testing MCPI-Reborn.

## Environmental Variables

### ``MCPI_DEBUG``
This enables debug logging if it is set.

### ``MCPI_API_PORT``
This configures the API to use a different port (the default is 4711).

### Client Mode Only
If any of the following variables aren't set, one configuration dialog will open on startup for each unset variable.

#### ``MCPI_FEATURE_FLAGS``
This corresponds to ``--print-available-feature-flags``. This is just a list of all enabled feature flags separated by ``|``.

For instance, the string ``Feature A|Feature B`` would enable both ``Feature A`` and ``Feature B`` and *disable every other available feature flag*.

#### ``MCPI_RENDER_DISTANCE``
This is the render distance. The possible values are: ``Far``, ``Normal``, ``Short``, and ``Tiny``.

#### ``MCPI_USERNAME``
This is the username.
