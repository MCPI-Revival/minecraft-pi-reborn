#!/bin/sh

set -e
cd "$(dirname "$0")/../"

# Scripts
find_sh() {
    # shellcheck disable=SC2150
    find . \
        -type f \
        -name '*.sh' \
        -exec "$@" {} +
}
find_sh shellcheck \
    --check-sourced \
    --external-sources \
    --source-path=SCRIPTDIR \
    --shell=sh \
    --enable=all
find_sh checkbashisms --force

# Markdown
cd ../
STYLE="$(mktemp --tmpdir "markdown-lint.XXXXXXXXXX.toml")"
style() {
    echo "$1" >> "${STYLE}"
}
style 'default = true'
style 'blanks-around-headings.lines_below = 0'
style 'line-length = false'
style "no-inline-html.allowed_elements = ['b', 'ins', 'code', 'kbd', 'details', 'summary', 'p', 'img', 'a']"
style 'blanks-around-tables = false'
style 'blanks-around-fences = false'
style 'no-duplicate-heading.siblings_only = true'
style 'descriptive-link-text = false'
find docs mods/src README.md \
    -type f \
    -name '*.md' \
    -exec markdownlint \
        --config "${STYLE}" \
        {} +
rm -f "${STYLE}"