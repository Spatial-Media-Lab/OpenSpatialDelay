#!/bin/bash
# Install a specific version of OpenSpatialDelay from archived pre-built binaries
# Usage: ./scripts/install_version.sh v0.1
#        ./scripts/install_version.sh v0.2
#        ./scripts/install_version.sh v0.3

set -e

VERSION="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ARCHIVE_DIR="${PROJECT_DIR}/Archive/${VERSION}/builds/macOS"

if [ -z "$VERSION" ]; then
    echo "Usage: $0 <version>"
    echo "  e.g. $0 v0.1"
    echo ""
    echo "Available versions:"
    for d in "${PROJECT_DIR}"/Archive/v*/builds/macOS; do
        [ -d "$d" ] && echo "  $(basename "$(dirname "$(dirname "$d")")")"
    done
    exit 1
fi

if [ ! -d "$ARCHIVE_DIR" ]; then
    echo "ERROR: No pre-built binaries found for ${VERSION}"
    echo "Expected: ${ARCHIVE_DIR}"
    exit 1
fi

PLUGIN_NAME="OpenSpatialDelay ${VERSION}"
AU_SRC="${ARCHIVE_DIR}/${PLUGIN_NAME}.component"
VST3_SRC="${ARCHIVE_DIR}/${PLUGIN_NAME}.vst3"
AU_DEST="$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"

# Install AU
if [ -d "$AU_SRC" ]; then
    mkdir -p "$HOME/Library/Audio/Plug-Ins/Components"
    rm -rf "$AU_DEST"
    cp -R "$AU_SRC" "$AU_DEST"
    echo "AU installed: ${AU_DEST}"
else
    echo "WARNING: AU not found at ${AU_SRC}"
fi

# Install VST3
if [ -d "$VST3_SRC" ]; then
    mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3"
    rm -rf "$VST3_DEST"
    cp -R "$VST3_SRC" "$VST3_DEST"
    echo "VST3 installed: ${VST3_DEST}"
else
    echo "WARNING: VST3 not found at ${VST3_SRC}"
fi

echo "Done. Restart your DAW to pick up changes."
