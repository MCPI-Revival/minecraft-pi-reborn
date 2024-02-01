# Building

## Dependencies

### Debian/Ubuntu
```sh
./scripts/install-dependencies.sh
```

## Instructions
```sh
./scripts/build.sh <client|server> <armhf|arm64|i686|amd64>
```

### Custom CMake Arguments
```sh
./scripts/setup.sh <client|server> <armhf|arm64|i686|amd64> <Custom CMake Arguments>
./scripts/build.sh <client|server> <armhf|arm64|i686|amd64>
```

### Environment Variables
* `MCPI_TOOLCHAIN_USE_DEFAULT_SEARCH_PATHS`: Use Default CMake Search Paths Rather Than Guessing
