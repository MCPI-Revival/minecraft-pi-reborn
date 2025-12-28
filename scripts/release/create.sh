#!/bin/sh

# Setup
set -e
cd "$(dirname "$0")"
. ./common.sh

# Arguments
TAG="$1"

# Setup Gitea CLI
TEA_VERSION='0.10.1'
TEA="$(mktemp --tmpdir "tea-${TEA_VERSION}.XXXXXXXXXX")"
curl -o "${TEA}" "https://dl.gitea.com/tea/${TEA_VERSION}/tea-${TEA_VERSION}-linux-amd64"
chmod +x "${TEA}"

# Login
LOGIN_NAME='ci'
"${TEA}" logins delete "${LOGIN_NAME}" || :
"${TEA}" logins add \
    --name "${LOGIN_NAME}" \
    --url "${SERVER}" \
    --token "${RELEASE_TOKEN:?}"
"${TEA}" logins default "${LOGIN_NAME}"

# Create Release
CHANGELOG="$(URL="${SERVER}/${SLUG}/src/tag/${TAG}" ./scripts/misc/get-changelog.mjs release)"
"${TEA}" releases create \
    --repo "${SLUG}" \
    --tag "${TAG}" \
    --title "v${VERSION}" \
    --note "${CHANGELOG}"

# Attachments
find ./out \
    -type f \
    -and -not -name '*.so' \
    -exec "${TEA}" releases assets create --repo "${SLUG}" "${TAG}" {} +

# Clean Up
rm -f "${TEA}"