---
phase: 01-repo-license-readiness
plan: "04"
subsystem: legal
tags: [license, gpl-3.0, compliance, release-packaging, gap-closure]
dependency_graph:
  requires: [REPO-01]
  provides: [REPO-02]
  affects:
    - docs/generate_legal_notices.js
    - docs/OpenSpatialDelay_Legal_Notices.pdf
    - scripts/package_macos_release.sh
    - .gitignore
tech_stack:
  added: []
  patterns:
    - "DOCX → PDF conversion via `soffice --headless --convert-to pdf` shelled out from Node"
    - "Staging-only release packager (build externally, package separately)"
key_files:
  created:
    - docs/OpenSpatialDelay_Legal_Notices.pdf
    - scripts/package_macos_release.sh
  modified:
    - docs/generate_legal_notices.js
    - .gitignore
decisions:
  - "PDF export via LibreOffice `soffice` shell-out rather than adding a new npm PDF library (matches repo's external-tool pattern for the manual PDF and keeps the dependency footprint minimal)"
  - "Packaging script does not build — expects caller to populate `dist/` via `build_version.sh` first. Separation of concerns: build vs. package"
  - "Legal/ folder is staged inside `dist/` (then zipped at the root of the archive), so the archive's top level mirrors the installer layout users see"
  - "T3 (release asset replacement on GitHub) is a user-gated checkpoint — no automated `gh release upload --clobber` from an agent"
metrics:
  duration: "~20 minutes executable work (prior commits e815312 + d63e7f5); ~15 minutes verification + summary on resumption"
  completed: "2026-04-15 (T1+T2), T3 pending user checkpoint"
  tasks_completed: 2
  tasks_total: 3
  files_modified: 2
  files_created: 2
---

# Phase 01 Plan 04: Release Packaging + Legal Notices Bundle (Gap G-02) Summary

**One-liner:** Added PDF export to the legal notices generator and a repeatable macOS release packager that bundles `Legal/LICENSE.txt` + `Legal/OpenSpatialDelay_Legal_Notices.pdf` alongside the VST3/AU payload, closing GPL-3.0 §6 and third-party attribution gap G-02 for the binary v1.0.0 release. Asset-replacement on GitHub is held for user-gated checkpoint.

## Resumption Context

This execution resumed after commits `e815312` (T1) and `d63e7f5` (T2) had already been authored on `main`. The executor re-verified all acceptance criteria from those commits against the current worktree state and produced the SUMMARY. No additional code changes were required for T1 or T2 — they verified clean.

T3 (the `checkpoint:human-verify` task — destructive `gh release upload --clobber`) is held as a checkpoint per plan's `autonomous: false` flag. Returned structurally rather than executed.

## What Was Built

### Task 01-04-T1 — PDF export in legal notices generator  ✓ (prior commit `e815312`)

Added to `docs/generate_legal_notices.js`:

```js
const { execSync } = require("child_process");
// ...
const OUTPUT_PDF_PATH = path.join(DOCS_DIR, "OpenSpatialDelay_Legal_Notices.pdf");
// ...
// (at tail, after fs.writeFileSync(OUTPUT_PATH, buffer))
try {
  execSync(
    `soffice --headless --convert-to pdf "${OUTPUT_PATH}" --outdir "${DOCS_DIR}"`,
    { stdio: "inherit" }
  );
  console.log(`Wrote ${OUTPUT_PDF_PATH}`);
} catch (err) {
  console.error("PDF conversion failed. Is LibreOffice installed and `soffice` on PATH?");
  console.error("macOS install: brew install --cask libreoffice");
  throw err;
}
```

Re-ran the generator on this worktree:

```
Legal notices generated: .../docs/OpenSpatialDelay_Legal_Notices.docx
File size: 51.3 KB
convert .../docs/OpenSpatialDelay_Legal_Notices.docx as a Writer document
  -> .../docs/OpenSpatialDelay_Legal_Notices.pdf
  using filter : writer_pdf_Export
Overwriting: .../docs/OpenSpatialDelay_Legal_Notices.pdf
Wrote .../docs/OpenSpatialDelay_Legal_Notices.pdf
```

**PDF file:** `docs/OpenSpatialDelay_Legal_Notices.pdf`, 120,122 bytes, regenerated at 16:00:58 local time.

**Sample extracted text (pdftotext):**
- `EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF` (MIT boilerplate)
- `Apache License` (for Roboto / SADIE per project docs)
- `CC BY 4.0`
- `CC BY 3.0` (for HUTUBS)

Commit: `e815312`

### Task 01-04-T2 — `scripts/package_macos_release.sh`  ✓ (prior commit `d63e7f5`)

Creates a repeatable macOS arm64 release packager that stages `Legal/` and zips. Full content (already in tree, 1,586 bytes, executable):

```bash
#!/usr/bin/env bash
# Package a macOS arm64 release ZIP with required legal notices.
# Expects dist/ to already contain the built VST3 + AU payload.
# This script DOES NOT build — it only stages Legal/ and zips.
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

if [[ ! -d "$DIST_DIR" ]]; then
  echo "ERROR: $DIST_DIR does not exist. Build and stage VST3/AU first." >&2
  exit 1
fi
if ! find "$DIST_DIR" -maxdepth 4 \( -name "*.vst3" -o -name "*.component" \) | grep -q .; then
  echo "ERROR: No .vst3 or .component found under $DIST_DIR. Build the plugin first." >&2
  exit 1
fi
[[ -f "$LEGAL_PDF"   ]] || { echo "ERROR: missing $LEGAL_PDF — run node docs/generate_legal_notices.js first" >&2; exit 1; }
[[ -f "$LICENSE_SRC" ]] || { echo "ERROR: missing $LICENSE_SRC" >&2; exit 1; }

mkdir -p "$LEGAL_DIR"
cp "$LEGAL_PDF"   "$LEGAL_DIR/OpenSpatialDelay_Legal_Notices.pdf"
cp "$LICENSE_SRC" "$LEGAL_DIR/LICENSE.txt"

rm -f "$ZIP_PATH"
(cd "$DIST_DIR" && zip -r "$ZIP_PATH" .)

echo ""
echo "=== $ZIP_PATH contents ==="
unzip -l "$ZIP_PATH"
```

Commit: `d63e7f5` (also added `dist/` and `OpenSpatialDelay-*.zip` to `.gitignore`).

#### Smoke test run (this execution, with mock `dist/` payload)

Created `dist/OpenSpatialDelay.vst3/Contents/Resources/plugin.bin` and `dist/OpenSpatialDelay.component/Contents/Info.plist` to simulate a post-build tree, then ran:

```
bash scripts/package_macos_release.sh v1.0.0-test
```

Result ZIP (`unzip -l OpenSpatialDelay-v1.0.0-test-macOS-arm64.zip`):

```
Archive:  OpenSpatialDelay-v1.0.0-test-macOS-arm64.zip
  Length      Date    Time    Name
---------  ---------- -----   ----
        0  04-15-2026 16:01   OpenSpatialDelay.component/
        0  04-15-2026 16:01   OpenSpatialDelay.component/Contents/
       16  04-15-2026 16:01   OpenSpatialDelay.component/Contents/Info.plist
        0  04-15-2026 16:01   OpenSpatialDelay.vst3/
        0  04-15-2026 16:01   OpenSpatialDelay.vst3/Contents/
        0  04-15-2026 16:01   OpenSpatialDelay.vst3/Contents/Resources/
       20  04-15-2026 16:01   OpenSpatialDelay.vst3/Contents/Resources/plugin.bin
        0  04-15-2026 16:01   Legal/
   120122  04-15-2026 16:01   Legal/OpenSpatialDelay_Legal_Notices.pdf
    35684  04-15-2026 16:01   Legal/LICENSE.txt
---------                     -------
   155842                     10 files
```

All T2 acceptance criteria verified:
- `test -x scripts/package_macos_release.sh` → PASS (executable bit set)
- `bash -n scripts/package_macos_release.sh` → exit 0 (syntax valid)
- Positive run with populated `dist/` → exit 0, ZIP produced at repo root
- `Legal/LICENSE.txt` and `Legal/OpenSpatialDelay_Legal_Notices.pdf` both present
- VST3 + .component payload intact (regression check)
- Negative test `dist/` missing → `ERROR: ... does not exist. Build and stage VST3/AU first.` exit 1
- Negative test `dist/` empty → `ERROR: No .vst3 or .component found under ...` exit 1

Smoke artifacts (mock `dist/` + test ZIP) removed after verification; working tree clean relative to `d63e7f5`.

### Task 01-04-T3 — CHECKPOINT (user-gated, not executed)

**Status:** PENDING — `checkpoint:human-verify` per plan.

**What's needed from user:**
1. Run a real build: `bash scripts/build_version.sh <hash> v1.0.0 O100` (or equivalent), ensuring `dist/` is populated with production VST3 + AU.
2. Run `bash scripts/package_macos_release.sh v1.0.0` to produce the replacement ZIP.
3. Review `unzip -l OpenSpatialDelay-v1.0.0-macOS-arm64.zip` output; confirm `Legal/LICENSE.txt` + `Legal/OpenSpatialDelay_Legal_Notices.pdf` are present alongside real VST3/AU.
4. Explicitly approve the clobber upload (the existing v1.0.0 asset on GitHub, size 75,198,970 bytes, updated `2026-04-13T14:00:52Z`, will be replaced).
5. Execute: `gh release upload v1.0.0 OpenSpatialDelay-v1.0.0-macOS-arm64.zip --clobber`
6. Verify via `gh release view v1.0.0 --json assets` that the `updatedAt` timestamp on the macOS asset is fresh.

**Current GitHub v1.0.0 asset (compliance-broken, pre-fix):**
- `OpenSpatialDelay-v1.0.0-macOS-arm64.zip` — 75,198,970 bytes, `updatedAt: 2026-04-13T14:00:52Z`, sha256 `24c124545b27720c7cb58d136f62870e40728ca267159e3d9a2d90886bea2298`
- (Source ZIP asset is unaffected and remains.)

No automation performed for this task. No `gh` mutation calls have been made.

## Deviations from Plan

None. Plan executed as written. The only departure is that T1 and T2 landed in prior commits (`e815312`, `d63e7f5`) during an earlier iteration; this execution re-verified acceptance criteria and authored the SUMMARY. T3 remains a checkpoint per plan design.

## Authentication Gates

None. T3's `gh release upload` would require an authenticated `gh` session, but it is intentionally held for the user per the plan's checkpoint contract — not executed automatically.

## Known Stubs

None. All artifacts are real: the PDF is a 120KB file containing complete third-party notices, the packager is a working shell script that produces a verified-correct ZIP layout.

## Self-Check: PASSED

Verified claims:

- `docs/generate_legal_notices.js` — FOUND, contains `execSync`, `OUTPUT_PDF_PATH`, and `soffice --headless --convert-to pdf`
- `docs/OpenSpatialDelay_Legal_Notices.pdf` — FOUND (120,122 bytes, regenerated successfully via generator)
- `scripts/package_macos_release.sh` — FOUND (executable, `bash -n` passes, smoke test exit 0, negative tests exit 1 with clear errors)
- Commit `e815312` — FOUND in `git log`
- Commit `d63e7f5` — FOUND in `git log`
- Acceptance: PDF text contains `Apache License`, `CC BY 4.0`, `CC BY 3.0`, MIT license boilerplate phrase — CONFIRMED via `pdftotext`
- T3: Intentionally not executed — GitHub v1.0.0 macOS asset still shows `updatedAt: 2026-04-13T14:00:52Z` (pre-fix)
