#!/bin/sh

set -e

# Get Root Directory
ROOT="$(realpath "$(dirname "$0")")"
CONFIGS="${ROOT}/configs"
SCRIPTS_ROOT="${ROOT}/../../"
REPO_ROOT="${SCRIPTS_ROOT}/../"

# Setup NPM
npm ci --prefix "${ROOT}"
NODE_MODULES="${ROOT}/node_modules"
PATH="${NODE_MODULES}/.bin:${PATH}"

# Utility Function
find_and_run() {
    EXT="$1"
    shift
    DIR="$(pwd)"
    # shellcheck disable=SC2150
    find "${DIR}" \
        -path "${NODE_MODULES}" -prune -o \
        -type f \
        -name "*.${EXT}" \
        -exec "$@" {} +
}
find_sh() {
    find_and_run sh "$@"
}
find_js() {
    find_and_run mjs "$@"
}

# Scripts
cd "${SCRIPTS_ROOT}"
find_sh shellcheck \
    --check-sourced \
    --rcfile "${CONFIGS}/shellcheckrc"
find_sh checkbashisms \
    --force \
    --extra \
    --posix
find_js eslint \
    --config "${CONFIGS}/eslint.config.mjs"
find_js tsc \
    --allowJs --checkJs \
    --noEmit \
    --module nodenext \
    --target ES2022 \
    --typeRoots "${NODE_MODULES}/@types" \
    --types node

# Markdown
cd "${REPO_ROOT}"
find docs mods/src README.md \
    -type f \
    -name '*.md' \
    -exec markdownlint \
        --config "${CONFIGS}/markdownlint.toml" \
        {} +