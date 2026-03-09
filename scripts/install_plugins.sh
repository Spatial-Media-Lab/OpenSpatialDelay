#!/bin/bash
# Post-build plugin installer for macOS
# Called automatically by CMake after each build

PLUGIN_NAME="OpenSpatialDelay v0.3"
BUILD_DIR="$1"
AU_SRC="${BUILD_DIR}/OpenSpatialDelay_artefacts/Release/AU/${PLUGIN_NAME}.component"
VST3_SRC="${BUILD_DIR}/OpenSpatialDelay_artefacts/Release/VST3/${PLUGIN_NAME}.vst3"
AU_DEST="$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"

# Install AU
if [ -d "$AU_SRC" ]; then
    mkdir -p "$HOME/Library/Audio/Plug-Ins/Components"
    rm -rf "$AU_DEST"
    cp -R "$AU_SRC" "$AU_DEST"
    echo "AU installed to $AU_DEST"
fi

# Install VST3
if [ -d "$VST3_SRC" ]; then
    VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
    if [ -w "$VST3_DIR" ]; then
        rm -rf "$VST3_DEST"
        cp -R "$VST3_SRC" "$VST3_DEST"
        echo "VST3 installed to $VST3_DEST"
    else
        echo "WARNING: VST3 dir not writable. Run once:"
        echo "  sudo chown -R $(whoami):staff $VST3_DIR"
    fi
fi
