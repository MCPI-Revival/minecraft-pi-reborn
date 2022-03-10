# Command Line Usage

## Command Line Arguments

### ``--print-available-feature-flags`` (Client Mode Only)
If you run MCPI-Reborn with ``--print-available-feature-flags``, it will print the available feature flags and then immediately exit.

The feature flags are printed in the following format:
```
TRUE This Flag Is On By Default
FALSE This Flag Is Off By Default
```

### ``--only-generate`` (Server Mode Only)
If you run MCPI-Reborn with ``--only-generate``, it will immediately exit once world generation has completed. This is mainly used for automatically testing MCPI-Reborn.

### ``--benchmark`` (Client Mode Only)
If you run MCPI-Reborn with ``--benchmark``, it will enter a simple benchmark mode. This means automatically loading a newly generated world, then rotating the camera for a period of time. When it has finished, it will then exit and print the average FPS while the world was loaded. In this mode, all user input is blocked. However you can still modify rendering settings by changing feature flags.

The world used will always be re-created on start and uses a hard-coded seed.

## Environmental Variables

### ``MCPI_DEBUG``
This enables debug logging if you set it to any non-zero-length value.

### Client Mode Only
If a value isn't set for any of the following variables, a GUI will open that allows you to select one.

### ``MCPI_FEATURE_FLAGS``
This corresponds to ``--print-available-feature-flags``. This is just a list of all enabled feature flags separated by ``|``.

For instance, the string ``Feature A|Feature B`` would enable both ``Feature A`` and ``Feature B`` and *disable every other available feature flag*.

### ``MCPI_RENDER_DISTANCE``
This is the render distance. The possible values are: ``Far``, ``Normal``, ``Short``, and ``Tiny``.

### ``MCPI_USERNAME``
This is the username.
