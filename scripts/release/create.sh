#!/bin/sh

# Setup
set -e
cd "$(dirname "$0")"
. ./common.sh

# Upload Packages To Stable Repository
./scripts/release/upload-packages.sh stable

# Arguments
TAG="$1"

# Setup Gitea CLI
TEA_VERSION='0.10.1'
TEA=/tmp/tea
curl -o "${TEA}" "https://dl.gitea.com/tea/${TEA_VERSION}/tea-${TEA_VERSION}-linux-amd64"
chmod +x "${TEA}"

# Login
LOGIN_NAME='ci'
"${TEA}" logins delete "${LOGIN_NAME}" || :
"${TEA}" logins add \
    --name "${LOGIN_NAME}" \
    --url "${SERVER}" \
    --token "${RELEASE_TOKEN}"
"${TEA}" logins default "${LOGIN_NAME}"

# Create Release
"${TEA}" releases create \
    --repo "${SLUG}" \
    --tag "${TAG}" \
    --title "v${VERSION}" \
    --note "$(./scripts/get-changelog.mjs release)"

# Attachments
find ./out \
    -type f \
    -and -not -name '*.so' \
    -exec "${TEA}" releases assets create "${TAG}" {} --repo "${SLUG}" \;