#!/bin/bash
# Download latest Windows VST3 build from GitHub Actions
# Requires: gh CLI (brew install gh) and authenticated (gh auth login)
#
# Usage:
#   bash scripts/download_windows_build.sh
#   cmake --build build --target download_windows_build

set -e

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DEST_DIR="${REPO_DIR}/build/windows"
ARTIFACT_NAME="OpenSpatialDelay-Windows-VST3"

# Check gh CLI is available
if ! command -v gh &> /dev/null; then
    echo "ERROR: GitHub CLI (gh) is not installed."
    echo "  Install with: brew install gh"
    echo "  Then authenticate: gh auth login"
    exit 1
fi

# Check authentication
if ! gh auth status &> /dev/null 2>&1; then
    echo "ERROR: GitHub CLI is not authenticated."
    echo "  Run: gh auth login"
    exit 1
fi

# Get repo name
REPO=$(gh repo view --json nameWithOwner -q .nameWithOwner 2>/dev/null)
if [ -z "$REPO" ]; then
    echo "ERROR: Could not determine GitHub repository."
    echo "  Make sure you have a remote configured: git remote add origin <url>"
    exit 1
fi

echo "Fetching latest Windows VST3 build from ${REPO}..."

# Find the latest successful run of the Windows build workflow
RUN_ID=$(gh run list --workflow="build-windows.yml" --status=success --limit=1 --json databaseId -q '.[0].databaseId' --repo "$REPO" 2>/dev/null)

if [ -z "$RUN_ID" ]; then
    echo "ERROR: No successful Windows build found."
    echo "  Push to main branch to trigger a build, or run manually:"
    echo "  gh workflow run build-windows.yml --repo $REPO"
    exit 1
fi

echo "Found successful run: #${RUN_ID}"

# Clean previous download
rm -rf "${DEST_DIR}"
mkdir -p "${DEST_DIR}"

# Download the artifact
gh run download "$RUN_ID" --name "$ARTIFACT_NAME" --dir "${DEST_DIR}" --repo "$REPO"

echo ""
echo "Windows VST3 downloaded to:"
echo "  ${DEST_DIR}"
echo ""
echo "To install on Windows, copy the .vst3 folder to:"
echo "  C:\Program Files\Common Files\VST3\\"
