#!/usr/bin/env bash
# Package a macOS arm64 release ZIP with the standard staging layout:
#
#   OpenSpatialDelay-macOS-arm64.zip          (unversioned filename — stable CDN URL)
#   └── OpenSpatialDelay-vX.Y.Z/              (versioned top-level folder)
#       ├── OpenSpatialDelay.component        (unversioned bundle)
#       ├── OpenSpatialDelay.vst3             (unversioned bundle)
#       ├── Install-OpenSpatialDelay.command
#       ├── Remove-Quarantine-macOS.command
#       ├── README.txt
#       ├── OpenSpatialDelay_Manual_v1.0.pdf
#       └── Legal/
#
# Expects dist/ to already contain the built VST3 + AU bundles at the
# top level (where install_plugins.sh / build_version.sh leave them).
# This script DOES NOT build — it only stages and zips.
#
# Usage: bash scripts/package_macos_release.sh v1.0.0
#
# See agent_docs/version_registry.md for the installer-revision counter
# (installer-r1, installer-r2, ...) that increments when packaging-only
# changes ship without a plugin binary rebuild.

set -euo pipefail

VERSION="${1:-}"
if [[ -z "$VERSION" ]]; then
  echo "Usage: $0 <version>   (e.g. v1.0.0)" >&2
  exit 2
fi

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIST_DIR="$REPO_ROOT/dist"
STAGE_NAME="OpenSpatialDelay-$VERSION"
STAGE_DIR="$DIST_DIR/$STAGE_NAME"
ZIP_PATH="$REPO_ROOT/OpenSpatialDelay-macOS-arm64.zip"

LEGAL_PDF="$REPO_ROOT/docs/OpenSpatialDelay_Legal_Notices.pdf"
LICENSE_SRC="$REPO_ROOT/LICENSE"
MANUAL_PDF="$REPO_ROOT/docs/OpenSpatialDelay_Manual_v1.0.pdf"
README_SRC="$REPO_ROOT/scripts/release_assets/README-macOS.txt"
INSTALLER_HELPER="$REPO_ROOT/scripts/release_assets/Install-OpenSpatialDelay.command"
QUARANTINE_HELPER="$REPO_ROOT/scripts/release_assets/Remove-Quarantine-macOS.command"

# Locate built bundles in dist/ (top level — both unversioned and
# legacy-versioned names are accepted, first match wins).
shopt -s nullglob
COMPONENT_MATCHES=( "$DIST_DIR"/OpenSpatialDelay*.component )
VST3_MATCHES=( "$DIST_DIR"/OpenSpatialDelay*.vst3 )
shopt -u nullglob

if [[ ${#COMPONENT_MATCHES[@]} -eq 0 ]]; then
  echo "ERROR: No OpenSpatialDelay*.component found at top level of $DIST_DIR." >&2
  echo "Build the plugin and ensure the .component is staged in dist/." >&2
  exit 1
fi
if [[ ${#VST3_MATCHES[@]} -eq 0 ]]; then
  echo "ERROR: No OpenSpatialDelay*.vst3 found at top level of $DIST_DIR." >&2
  echo "Build the plugin and ensure the .vst3 is staged in dist/." >&2
  exit 1
fi
COMPONENT_SRC="${COMPONENT_MATCHES[0]}"
VST3_SRC="${VST3_MATCHES[0]}"

# Guard: required source files must exist.
[[ -f "$LEGAL_PDF"          ]] || { echo "ERROR: missing $LEGAL_PDF — run node docs/generate_legal_notices.js first" >&2; exit 1; }
[[ -f "$LICENSE_SRC"        ]] || { echo "ERROR: missing $LICENSE_SRC" >&2; exit 1; }
[[ -f "$MANUAL_PDF"         ]] || { echo "ERROR: missing $MANUAL_PDF — regenerate manual via docs/generate_manual.js + soffice" >&2; exit 1; }
[[ -f "$README_SRC"         ]] || { echo "ERROR: missing $README_SRC" >&2; exit 1; }
[[ -x "$INSTALLER_HELPER"   ]] || { echo "ERROR: missing or non-executable $INSTALLER_HELPER" >&2; exit 1; }
[[ -x "$QUARANTINE_HELPER"  ]] || { echo "ERROR: missing or non-executable $QUARANTINE_HELPER" >&2; exit 1; }

# Build the staging tree from scratch for deterministic output.
rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/Legal"

# Plugin bundles — staged with unversioned wrapper names.
#
# CMakeLists currently builds bundles with versioned PRODUCT_NAME
# ("OpenSpatialDelay v1.0.0") so the DAW shows the version inline next to the
# plugin name. For shipping we strip the version from the four filename-shaped
# fields so v1.0.x can be upgraded in place by future v1.0.(x+1) builds:
#
#   - bundle wrapper directory name        (e.g. OpenSpatialDelay.component)
#   - inner binary filename                (Contents/MacOS/OpenSpatialDelay)
#   - Info.plist :CFBundleExecutable       (must match the binary filename)
#   - Info.plist :CFBundleIdentifier       (no .vX-Y-Z suffix → version-stable ID)
#
# What STAYS versioned (so the DAW still shows "v1.0.0" inline):
#   - CFBundleName, CFBundleDisplayName    (Finder display)
#   - AU AudioComponents[0].name           (AU host display)
#   - AU AudioComponents[0].description    (AU host display)
#   - AU AudioComponents[0].factoryFunction (MUST match symbol baked into binary —
#                                            do NOT touch, or AU host can't load)
#   - JucePlugin_Name baked into binary    (DAW VST3 display name)
#
# When the About-popup work (issue #13) lands and PRODUCT_NAME drops the version
# suffix, this normalization becomes a no-op and the script keeps working.

normalize_macos_bundle() {
  local SRC="$1"  # source bundle path (versioned or unversioned)
  local DST="$2"  # destination path with unversioned name

  rm -rf "$DST"
  cp -R "$SRC" "$DST"

  local PLIST="$DST/Contents/Info.plist"
  [[ -f "$PLIST" ]] || { echo "ERROR: missing Info.plist at $PLIST" >&2; return 1; }

  # 1. Rename inner binary to unversioned ("OpenSpatialDelay").
  local MACOS_DIR="$DST/Contents/MacOS"
  if [[ -d "$MACOS_DIR" ]]; then
    local INNER_BIN
    INNER_BIN="$(find "$MACOS_DIR" -maxdepth 1 -mindepth 1 -type f | head -n 1)"
    if [[ -n "$INNER_BIN" ]]; then
      local INNER_NAME
      INNER_NAME="$(basename "$INNER_BIN")"
      if [[ "$INNER_NAME" != "OpenSpatialDelay" ]]; then
        mv "$INNER_BIN" "$MACOS_DIR/OpenSpatialDelay"
      fi
    fi
  fi

  # 2. Patch CFBundleExecutable to match the renamed binary.
  /usr/libexec/PlistBuddy -c "Set :CFBundleExecutable OpenSpatialDelay" "$PLIST" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleExecutable string OpenSpatialDelay" "$PLIST"

  # 3. Force-set CFBundleIdentifier to the canonical release value. Handles
  #    every input variant defensively:
  #      - vanilla cmake:      com.Spatial Media Lab.OpenSpatialDelay  (spaces, JUCE-warned)
  #      - build_version.sh:   com.spatialmedialab.OpenSpatialDelay.v1-0-0  (PlistBuddy-patched)
  #    Future v1.0.x builds get the same ID → DAWs treat as upgrade-in-place.
  /usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier com.spatialmedialab.OpenSpatialDelay" "$PLIST"

  # 4. Re-sign ad-hoc — required because we renamed the binary and edited the plist;
  #    the original ad-hoc signature is now invalid.
  xattr -cr "$DST"
  codesign --force --deep --sign - "$DST"
  codesign --verify --deep --strict "$DST"
}

normalize_macos_bundle "$COMPONENT_SRC" "$STAGE_DIR/OpenSpatialDelay.component"
normalize_macos_bundle "$VST3_SRC"      "$STAGE_DIR/OpenSpatialDelay.vst3"

# Shape check — fails loud rather than shipping a broken bundle.
echo ""
echo "=== Normalized bundle shape check ==="
for B in "$STAGE_DIR/OpenSpatialDelay.component" "$STAGE_DIR/OpenSpatialDelay.vst3"; do
  PLIST="$B/Contents/Info.plist"
  EXEC="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$PLIST")"
  ID="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' "$PLIST")"
  INNER="$(ls "$B/Contents/MacOS")"
  printf '  %-40s exec=%s  inner=%s\n  %-40s id=%s\n' \
    "$(basename "$B"):" "$EXEC" "$INNER" "" "$ID"
  if [[ "$EXEC" != "OpenSpatialDelay" ]] || [[ "$INNER" != "OpenSpatialDelay" ]]; then
    echo "ERROR: bundle $B has versioned filename/CFBundleExecutable" >&2; exit 1
  fi
  if [[ "$ID" != "com.spatialmedialab.OpenSpatialDelay" ]]; then
    echo "ERROR: bundle ID is $ID — expected com.spatialmedialab.OpenSpatialDelay" >&2; exit 1
  fi
done

# Helper scripts (preserve exec bit).
cp "$INSTALLER_HELPER"  "$STAGE_DIR/Install-OpenSpatialDelay.command"
cp "$QUARANTINE_HELPER" "$STAGE_DIR/Remove-Quarantine-macOS.command"
chmod +x "$STAGE_DIR/Install-OpenSpatialDelay.command" \
         "$STAGE_DIR/Remove-Quarantine-macOS.command"

# Top-level docs.
cp "$README_SRC" "$STAGE_DIR/README.txt"
cp "$MANUAL_PDF" "$STAGE_DIR/OpenSpatialDelay_Manual_v1.0.pdf"

# Legal/.
cp "$LEGAL_PDF"   "$STAGE_DIR/Legal/OpenSpatialDelay_Legal_Notices.pdf"
cp "$LICENSE_SRC" "$STAGE_DIR/Legal/LICENSE.txt"

# Produce ZIP — single top-level entry is the versioned folder.
rm -f "$ZIP_PATH"
(cd "$DIST_DIR" && zip -r "$ZIP_PATH" "$STAGE_NAME")

echo ""
echo "=== $ZIP_PATH contents ==="
unzip -l "$ZIP_PATH"

# Internal SHA-256 record (gitignored). Used for upload-vs-download
# verification round-trip; not published to users.
SHA_FILE="$REPO_ROOT/release-shasums.txt"
SHA_LINE="$(shasum -a 256 "$ZIP_PATH" | awk '{print $1}')  $(basename "$ZIP_PATH")  ($STAGE_NAME, packed $(date -u +%FT%TZ))"
touch "$SHA_FILE"
# Append (do not deduplicate — we want the history).
echo "$SHA_LINE" >> "$SHA_FILE"

echo ""
echo "SHA-256 recorded to $SHA_FILE:"
echo "  $SHA_LINE"
