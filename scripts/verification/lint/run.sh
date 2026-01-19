#!/bin/sh

set -e

# Get Root Directory
ROOT="$(realpath "$(dirname "$0")")"
SCRIPTS_ROOT="${ROOT}/../../"
REPO_ROOT="${SCRIPTS_ROOT}/../"

# Utility Function
find_and_run() {
    EXT="$1"
    shift
    # shellcheck disable=SC2150
    find . \
        -type f \
        -name "*.${EXT}" \
        -exec "$@" {} +
}
find_sh() {
    find_and_run sh "$@"
}

# Scripts
cd "${SCRIPTS_ROOT}"
find_sh shellcheck \
    --check-sourced \
    --rcfile "${ROOT}/shellcheckrc"
find_sh checkbashisms \
    --force \
    --extra \
    --posix
NODE_ROOT="$(npm root --quiet --global)"
NODE_MODULES="${REPO_ROOT}/node_modules"
ln --symbolic --force --no-dereference "${NODE_ROOT}" "${NODE_MODULES}"
find_and_run mjs eslint \
    --config "${ROOT}/eslint.config.mjs"
rm -f "${NODE_MODULES}"

# Markdown
cd "${REPO_ROOT}"
find docs mods/src README.md \
    -type f \
    -name '*.md' \
    -exec markdownlint \
        --config "${ROOT}/markdownlint.toml" \
        {} +