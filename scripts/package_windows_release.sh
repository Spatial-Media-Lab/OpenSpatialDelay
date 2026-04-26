#!/usr/bin/env bash
# Package a Windows x64 release ZIP with required legal notices.
#
# Expects build/windows/ to already contain the downloaded VST3 directory bundle
# (run scripts/download_windows_build.sh first to pull the latest successful
# build-windows.yml CI artifact).
#
# This script DOES NOT build — it only stages Legal/ alongside the .vst3 and zips.
# Auxiliary debug stubs from CI (.exp, .lib) are explicitly excluded from the
# release ZIP — only the .vst3 directory bundle ships.
#
# Usage: bash scripts/package_windows_release.sh v1.0.0

set -euo pipefail

VERSION="${1:-}"
if [[ -z "$VERSION" ]]; then
  echo "Usage: $0 <version>   (e.g. v1.0.0)" >&2
  exit 2
fi

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC_DIR="$REPO_ROOT/build/windows"
STAGE_DIR="$REPO_ROOT/dist-win"
LEGAL_DIR="$STAGE_DIR/Legal"
ZIP_PATH="$REPO_ROOT/OpenSpatialDelay-$VERSION-Windows-x64.zip"

LEGAL_PDF="$REPO_ROOT/docs/OpenSpatialDelay_Legal_Notices.pdf"
LICENSE_SRC="$REPO_ROOT/LICENSE"

# Guard: build/windows/ must contain the downloaded artifact.
if [[ ! -d "$SRC_DIR" ]]; then
  echo "ERROR: $SRC_DIR does not exist. Run scripts/download_windows_build.sh first." >&2
  exit 1
fi

# Locate the .vst3 directory bundle inside build/windows/.
VST3_BUNDLE="$(find "$SRC_DIR" -maxdepth 2 -name '*.vst3' -type d | head -n 1)"
if [[ -z "$VST3_BUNDLE" ]]; then
  echo "ERROR: No .vst3 directory bundle found under $SRC_DIR." >&2
  exit 1
fi

# Guard: legal sources must exist.
[[ -f "$LEGAL_PDF"   ]] || { echo "ERROR: missing $LEGAL_PDF — run node docs/generate_legal_notices.js first" >&2; exit 1; }
[[ -f "$LICENSE_SRC" ]] || { echo "ERROR: missing $LICENSE_SRC" >&2; exit 1; }

# Restage cleanly for deterministic output.
rm -rf "$STAGE_DIR"
mkdir -p "$LEGAL_DIR"

cp -R "$VST3_BUNDLE" "$STAGE_DIR/"
cp "$LEGAL_PDF"   "$LEGAL_DIR/OpenSpatialDelay_Legal_Notices.pdf"
cp "$LICENSE_SRC" "$LEGAL_DIR/LICENSE.txt"

# Produce ZIP. Excludes .exp/.lib debug stubs that the CI artifact also ships.
rm -f "$ZIP_PATH"
(cd "$STAGE_DIR" && zip -r "$ZIP_PATH" . -x '*.exp' '*.lib')

echo ""
echo "=== $ZIP_PATH contents ==="
unzip -l "$ZIP_PATH"
