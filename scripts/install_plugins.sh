#!/bin/bash
# Post-build plugin installer for macOS
# Called automatically by CMake after each build

PLUGIN_NAME="OpenSpatialDelay v1.0"
BUILD_DIR="$1"
AU_SRC="${BUILD_DIR}/OpenSpatialDelay_artefacts/Release/AU/${PLUGIN_NAME}.component"
VST3_SRC="${BUILD_DIR}/OpenSpatialDelay_artefacts/Release/VST3/${PLUGIN_NAME}.vst3"
AU_DEST="$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"

# v1.0.2: Validate AU binary exists before installing (not just the directory).
# JUCE creates the .component directory at configure time, but the binary is only
# written when the AU target finishes linking. Without this check, the script can
# copy an empty bundle with Info.plist but no actual plugin binary.
AU_BINARY="${AU_SRC}/Contents/MacOS/${PLUGIN_NAME}"
if [ -f "$AU_BINARY" ]; then
    mkdir -p "$HOME/Library/Audio/Plug-Ins/Components"
    rm -rf "$AU_DEST"
    cp -R "$AU_SRC" "$AU_DEST"
    xattr -cr "$AU_DEST" 2>/dev/null
    echo "AU installed to $AU_DEST"
else
    echo "WARNING: AU binary not found at $AU_BINARY — skipping AU install"
fi

# Install VST3
if [ -d "$VST3_SRC" ]; then
    VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
    if [ -w "$VST3_DIR" ]; then
        rm -rf "$VST3_DEST"
        cp -R "$VST3_SRC" "$VST3_DEST"
        xattr -cr "$VST3_DEST" 2>/dev/null
        echo "VST3 installed to $VST3_DEST"
    else
        echo "WARNING: VST3 dir not writable. Run once:"
        echo "  sudo chown -R $(whoami):staff $VST3_DIR"
    fi
fi

# v1.0: Factory presets are now compiled into the plugin binary.
# No disk-based preset installation needed.
