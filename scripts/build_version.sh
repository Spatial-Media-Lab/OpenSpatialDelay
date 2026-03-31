#!/bin/bash
# Build a specific git commit as a named plugin version with a unique plugin code.
# Creates clean builds in /tmp to avoid stale cache issues.
#
# Usage: ./scripts/build_version.sh <commit> <version> <plugin_code>
# Example: ./scripts/build_version.sh 023670a v1.0.0 O100
#
# The plugin will be installed to ~/Library/Audio/Plug-Ins/ as both AU and VST3.
# Each version gets a unique PLUGIN_CODE so multiple versions coexist in DAWs.

set -euo pipefail

if [ $# -ne 3 ]; then
    echo "Usage: $0 <commit> <version> <plugin_code>"
    echo "Example: $0 023670a v1.0.0 O100"
    exit 1
fi

COMMIT="$1"
VERSION="$2"
PLUGIN_CODE="$3"
PLUGIN_NAME="OpenSpatialDelay ${VERSION}"
REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="/tmp/osd-build-${VERSION}"
NCPU=$(sysctl -n hw.ncpu)

echo "=== Building ${PLUGIN_NAME} (commit ${COMMIT}, code ${PLUGIN_CODE}) ==="

# Step 1: Clean clone from local repo at specified commit
rm -rf "${BUILD_DIR}"
git clone --no-checkout "${REPO_DIR}" "${BUILD_DIR}" 2>/dev/null
cd "${BUILD_DIR}"
git checkout "${COMMIT}" -- . 2>/dev/null
git submodule update --init --recursive 2>/dev/null

# Step 2: Patch CMakeLists.txt with version-specific name, plugin code, and bundle ID
sed -i '' "s/PLUGIN_CODE Os10/PLUGIN_CODE ${PLUGIN_CODE}/" CMakeLists.txt
sed -i '' "s/PRODUCT_NAME \"OpenSpatialDelay v1.0\"/PRODUCT_NAME \"${PLUGIN_NAME}\"/" CMakeLists.txt

# v1.0.7: Compute unique bundle ID suffix for post-build plist patching (issue #62)
BUNDLE_SUFFIX=$(echo "${VERSION}" | tr '.' '-')  # e.g., v1.0.7 → v1-0-7
UNIQUE_BUNDLE_ID="com.SpatialMediaLibrary.OpenSpatialDelay.${BUNDLE_SUFFIX}"

# Step 3: Patch install script with version-specific name
sed -i '' "s/PLUGIN_NAME=\"OpenSpatialDelay v1.0\"/PLUGIN_NAME=\"${PLUGIN_NAME}\"/" scripts/install_plugins.sh

# Step 4: If commit predates the AU build dependency fix (925d9fe), inject it
if ! grep -q "add_dependencies(OpenSpatialDelay_VST3 OpenSpatialDelay_AU)" CMakeLists.txt; then
    echo "  Injecting AU build dependency (commit predates fix 925d9fe)..."
    # Insert add_dependencies before the add_custom_command line
    sed -i '' '/add_custom_command(TARGET OpenSpatialDelay_VST3 POST_BUILD/i\
    add_dependencies(OpenSpatialDelay_VST3 OpenSpatialDelay_AU)
' CMakeLists.txt
fi

# Step 5: Ensure install script validates AU binary (not just directory)
# Replace the directory check with a binary file check
if grep -q '\[ -d "\$AU_SRC" \]' scripts/install_plugins.sh; then
    echo "  Hardening install script AU validation..."
    cat > scripts/install_plugins.sh << 'INSTALL_EOF'
#!/bin/bash
PLUGIN_NAME="__PLUGIN_NAME__"
BUILD_DIR="$1"
AU_SRC="${BUILD_DIR}/OpenSpatialDelay_artefacts/Release/AU/${PLUGIN_NAME}.component"
VST3_SRC="${BUILD_DIR}/OpenSpatialDelay_artefacts/Release/VST3/${PLUGIN_NAME}.vst3"
AU_DEST="$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"
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
if [ -d "$VST3_SRC" ]; then
    VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
    mkdir -p "$VST3_DIR"
    rm -rf "$VST3_DEST"
    cp -R "$VST3_SRC" "$VST3_DEST"
    xattr -cr "$VST3_DEST" 2>/dev/null
    echo "VST3 installed to $VST3_DEST"
fi
INSTALL_EOF
    sed -i '' "s/__PLUGIN_NAME__/${PLUGIN_NAME}/" scripts/install_plugins.sh
fi

# Step 6: Clean cmake configure
echo "  Configuring..."
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 2>&1 | tail -1

# Step 7: Build AU first (must complete before install script runs)
echo "  Building AU..."
cmake --build build --target OpenSpatialDelay_AU -j${NCPU} 2>&1 | tail -1

# Step 8: Validate AU binary exists BEFORE building VST3 (which triggers install)
AU_BUILT="build/OpenSpatialDelay_artefacts/Release/AU/${PLUGIN_NAME}.component/Contents/MacOS/${PLUGIN_NAME}"
if [ ! -f "${AU_BUILT}" ]; then
    echo "ERROR: AU binary not found after build: ${AU_BUILT}"
    exit 1
fi
echo "  AU binary verified: $(file "${AU_BUILT}" | grep -o 'Mach-O.*')"

# Step 8b: Patch AU Info.plist with unique CFBundleIdentifier (issue #62)
# JUCE generates the same bundle ID for all builds; we must differentiate so macOS
# doesn't share/confuse resources when multiple versioned builds are loaded simultaneously.
AU_PLIST="build/OpenSpatialDelay_artefacts/Release/AU/${PLUGIN_NAME}.component/Contents/Info.plist"
if [ -f "${AU_PLIST}" ]; then
    /usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier ${UNIQUE_BUNDLE_ID}" "${AU_PLIST}"
    echo "  AU bundle ID patched: ${UNIQUE_BUNDLE_ID}"
fi

# Step 9: Build VST3 (triggers install script via POST_BUILD)
echo "  Building VST3 + installing..."
cmake --build build --target OpenSpatialDelay_VST3 -j${NCPU} 2>&1 | tail -3

# Step 9b: Patch installed AU and VST3 plists with unique CFBundleIdentifier
AU_INSTALLED_PLIST="$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component/Contents/Info.plist"
VST3_INSTALLED_PLIST="$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3/Contents/Info.plist"
if [ -f "${AU_INSTALLED_PLIST}" ]; then
    /usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier ${UNIQUE_BUNDLE_ID}" "${AU_INSTALLED_PLIST}"
fi
if [ -f "${VST3_INSTALLED_PLIST}" ]; then
    /usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier ${UNIQUE_BUNDLE_ID}" "${VST3_INSTALLED_PLIST}"
fi

# Step 10: Final validation of installed plugins
echo ""
echo "=== Validation ==="
AU_INSTALLED="$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
VST3_INSTALLED="$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"

PASS=true

# Check AU
AU_BIN_INSTALLED="${AU_INSTALLED}/Contents/MacOS/${PLUGIN_NAME}"
if [ -f "${AU_BIN_INSTALLED}" ]; then
    AU_SUB=$(/usr/libexec/PlistBuddy -c "Print :AudioComponents:0:subtype" "${AU_INSTALLED}/Contents/Info.plist" 2>/dev/null)
    AU_ARCH=$(file "${AU_BIN_INSTALLED}" | grep -o 'arm64\|x86_64')
    echo "  AU:   OK (subtype=${AU_SUB}, arch=${AU_ARCH})"
    if [ "${AU_SUB}" != "${PLUGIN_CODE}" ]; then
        echo "  ERROR: AU subtype '${AU_SUB}' does not match expected '${PLUGIN_CODE}'"
        PASS=false
    fi
else
    echo "  AU:   FAILED — binary missing"
    PASS=false
fi

# Check VST3
VST3_BIN_INSTALLED="${VST3_INSTALLED}/Contents/MacOS/${PLUGIN_NAME}"
if [ -f "${VST3_BIN_INSTALLED}" ]; then
    VST3_ARCH=$(file "${VST3_BIN_INSTALLED}" | grep -o 'arm64\|x86_64')
    echo "  VST3: OK (arch=${VST3_ARCH})"
else
    echo "  VST3: FAILED — binary missing"
    PASS=false
fi

if $PASS; then
    echo ""
    echo "=== ${PLUGIN_NAME} installed successfully ==="
else
    echo ""
    echo "=== ${PLUGIN_NAME} FAILED — see errors above ==="
    exit 1
fi
