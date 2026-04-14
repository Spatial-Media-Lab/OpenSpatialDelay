#!/usr/bin/env bash
# Package a macOS arm64 release ZIP with required legal notices.
#
# Expects dist/ to already contain the built VST3 + AU payload.
# This script DOES NOT build — it only stages Legal/ and zips.
#
# Usage: bash scripts/package_macos_release.sh v1.0.0

set -euo pipefail

VERSION="${1:-}"
if [[ -z "$VERSION" ]]; then
  echo "Usage: $0 <version>   (e.g. v1.0.0)" >&2
  exit 2
fi

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIST_DIR="$REPO_ROOT/dist"
LEGAL_DIR="$DIST_DIR/Legal"
ZIP_PATH="$REPO_ROOT/OpenSpatialDelay-$VERSION-macOS-arm64.zip"

LEGAL_PDF="$REPO_ROOT/docs/OpenSpatialDelay_Legal_Notices.pdf"
LICENSE_SRC="$REPO_ROOT/LICENSE"

# Guard: dist/ must already contain the built plugin payload.
if [[ ! -d "$DIST_DIR" ]]; then
  echo "ERROR: $DIST_DIR does not exist. Build and stage VST3/AU first." >&2
  exit 1
fi
if ! find "$DIST_DIR" -maxdepth 4 \( -name "*.vst3" -o -name "*.component" \) | grep -q .; then
  echo "ERROR: No .vst3 or .component found under $DIST_DIR. Build the plugin first." >&2
  exit 1
fi

# Guard: legal sources must exist.
[[ -f "$LEGAL_PDF"   ]] || { echo "ERROR: missing $LEGAL_PDF — run node docs/generate_legal_notices.js first" >&2; exit 1; }
[[ -f "$LICENSE_SRC" ]] || { echo "ERROR: missing $LICENSE_SRC" >&2; exit 1; }

# Stage Legal/ inside dist/.
mkdir -p "$LEGAL_DIR"
cp "$LEGAL_PDF"   "$LEGAL_DIR/OpenSpatialDelay_Legal_Notices.pdf"
cp "$LICENSE_SRC" "$LEGAL_DIR/LICENSE.txt"

# Produce ZIP.
rm -f "$ZIP_PATH"
(cd "$DIST_DIR" && zip -r "$ZIP_PATH" .)

echo ""
echo "=== $ZIP_PATH contents ==="
unzip -l "$ZIP_PATH"
