---
phase: 01-repo-license-readiness
reviewed: 2026-04-15T14:15:00Z
depth: standard
files_reviewed: 4
files_reviewed_list:
  - docs/generate_manual.js
  - docs/generate_legal_notices.js
  - scripts/package_macos_release.sh
  - .gitignore
findings:
  critical: 0
  warning: 2
  info: 5
  total: 7
status: issues_found
---

# Phase 01: Code Review Report (Gap-Closure Re-Review)

**Reviewed:** 2026-04-15T14:15:00Z
**Depth:** standard
**Files Reviewed:** 4
**Status:** issues_found

## Summary

This re-review scopes to the phase-01 gap-closure work (plans 01-03 and 01-04) that
split third-party legal notices out of the user manual into a standalone PDF and
added a macOS release packaging script. Diff base is `d30ddbb^..HEAD` covering
commits `517a39b`, `df5035e`, `e815312`, `d63e7f5`.

Overall the changes are small, focused, and safe. No critical security or
correctness issues were found. Two warnings cover a shell-injection antipattern
in the LibreOffice invocation and missing stale-state cleanup in the packager.
Five info-level items cover dead code, brittle error paths, and minor robustness
improvements. The packaging script uses `set -euo pipefail` correctly, quotes
interpolated paths throughout, and guards all required inputs before staging.

## Warnings

### WR-01: `execSync` with shell interpolation is a command-injection antipattern

**File:** `docs/generate_legal_notices.js:675-678`
**Issue:** The PDF conversion passes `OUTPUT_PATH` and `DOCS_DIR` directly into a
shell-interpreted command string via `execSync`. Both values are derived from
`__dirname`, so there is no external attacker surface today, but if the repo is
checked out to a path containing a double quote, dollar sign, or backtick (e.g.
`/Users/alice's work/openspatialdelay`), the command will either break or execute
unintended shell expansion. This is the textbook pattern flagged by code-quality
tools and makes the script needlessly fragile across developer machines.

**Fix:** Use `execFileSync` to bypass shell parsing entirely:

```js
const { execFileSync } = require("child_process");
// ...
try {
  execFileSync(
    "soffice",
    ["--headless", "--convert-to", "pdf", OUTPUT_PATH, "--outdir", DOCS_DIR],
    { stdio: "inherit" }
  );
  console.log(`Wrote ${OUTPUT_PDF_PATH}`);
} catch (err) {
  console.error("PDF conversion failed. Is LibreOffice installed and `soffice` on PATH?");
  console.error("macOS install: brew install --cask libreoffice");
  throw err;
}
```

### WR-02: Packager does not clear stale `dist/Legal/` before staging

**File:** `scripts/package_macos_release.sh:40-43`
**Issue:** `mkdir -p "$LEGAL_DIR"` followed by two `cp` commands will leave any
stale files from a previous packaging run inside `dist/Legal/`. If a filename
inside the legal payload ever changes (e.g. the PDF is renamed) or a developer
manually drops a file into `dist/Legal/` for testing, the resulting ZIP will
contain unintended content. Because the ZIP is produced with `zip -r .` from
`$DIST_DIR`, any stray file under `dist/Legal/` ships to end users.

**Fix:** Remove the staging directory before re-creating it, so each packaging
run is deterministic:

```bash
# Stage Legal/ inside dist/.
rm -rf "$LEGAL_DIR"
mkdir -p "$LEGAL_DIR"
cp "$LEGAL_PDF"   "$LEGAL_DIR/OpenSpatialDelay_Legal_Notices.pdf"
cp "$LICENSE_SRC" "$LEGAL_DIR/LICENSE.txt"
```

## Info

### IN-01: Dead code — `buildLegalNotices()` is defined but never called

**File:** `docs/generate_manual.js:1413`
**Issue:** Plan 01-03 removed the `...buildLegalNotices()` spread at line ~1687
and the TOC entry at line ~519, but left the ~270-line `buildLegalNotices()`
function definition in place. It is now orphaned dead code. Keeping it invites
drift: future edits to the third-party table in this function will silently have
no effect, and a reviewer glancing at the file may assume legal notices are still
in the manual.

**Fix:** Delete the `buildLegalNotices()` function (and the
`// SECTION 12: THIRD-PARTY NOTICES` banner comment that precedes it) from
`generate_manual.js`. The canonical source of this content now lives in
`generate_legal_notices.js`.

### IN-02: Packager writes ZIP to repo root rather than `dist/` output dir

**File:** `scripts/package_macos_release.sh:20`
**Issue:** `ZIP_PATH="$REPO_ROOT/OpenSpatialDelay-$VERSION-macOS-arm64.zip"` places
the release artifact at the repo root. `.gitignore` correctly excludes it with
`OpenSpatialDelay-v*-macOS-*.zip`, but putting build outputs at the repo root
mixes generated artifacts with source files and is inconsistent with the
established `build/` and `dist/` staging convention.

**Fix:** Write the ZIP to `dist/` (or a sibling `release/` directory) so all
generated artifacts live under a single ignored path:

```bash
ZIP_PATH="$DIST_DIR/../OpenSpatialDelay-$VERSION-macOS-arm64.zip"
# or
RELEASE_DIR="$REPO_ROOT/release"
mkdir -p "$RELEASE_DIR"
ZIP_PATH="$RELEASE_DIR/OpenSpatialDelay-$VERSION-macOS-arm64.zip"
```

If you keep the current location, at minimum document the output path in the
header comment so users know where to look.

### IN-03: VERSION argument is not validated for safe filename characters

**File:** `scripts/package_macos_release.sh:11-15`
**Issue:** `VERSION` is interpolated into a filename without format validation.
A developer typo like `bash scripts/package_macos_release.sh "v1.0 final"` or
`bash scripts/package_macos_release.sh /` produces a confusingly named ZIP
(`OpenSpatialDelay-v1.0 final-macOS-arm64.zip`) or writes to an unexpected path.
This is not an injection issue (quoting is correct), just a usability guard.

**Fix:** Validate against the semver-ish pattern used elsewhere in the repo:

```bash
if [[ ! "$VERSION" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "ERROR: VERSION must match vX.Y.Z (got: '$VERSION')" >&2
  exit 2
fi
```

### IN-04: `find -maxdepth 4` may miss nested plugin bundles

**File:** `scripts/package_macos_release.sh:31`
**Issue:** The guard `find "$DIST_DIR" -maxdepth 4 \( -name "*.vst3" -o -name
"*.component" \)` assumes the bundle sits within four directory levels of
`dist/`. If a developer stages under `dist/macOS/Release/VST3/Foo.vst3` (five
levels) the guard will false-negative and abort with "No .vst3 or .component
found" even though the payload is staged correctly. Depth 4 is probably fine for
the current layout, but the limit is silent and easy to trip.

**Fix:** Remove `-maxdepth 4` (the search is fast and `dist/` is small), or
raise it explicitly with a comment justifying the ceiling:

```bash
# -maxdepth 6 covers dist/macOS/Release/VST3/Name.vst3/Contents/... layouts
if ! find "$DIST_DIR" -maxdepth 6 \( -name "*.vst3" -o -name "*.component" \) -print -quit | grep -q .; then
  echo "ERROR: No .vst3 or .component found under $DIST_DIR. Build the plugin first." >&2
  exit 1
fi
```

Note also that the current form pipes `find` output to `grep -q .`, which works
but is less idiomatic than `-print -quit` (which exits `find` on the first
match).

### IN-05: Inconsistent error recovery — DOCX is kept on PDF failure

**File:** `docs/generate_legal_notices.js:668-684`
**Issue:** If `Packer.toBuffer` succeeds but `soffice` fails, the DOCX is written
and left on disk while the PDF step throws. The script's downstream consumer
(`package_macos_release.sh`) guards on the PDF, so the failure is caught — but
re-running the script will still regenerate the DOCX first, which is the
expected happy path. The minor inconsistency: the top-level `buildDocument()`
`catch` does `process.exit(1)` while the inner `catch` `throw`s, so the two
logging lines ("PDF conversion failed..." and "Error generating legal notices:")
are both printed for the same error. Not harmful, just noisy.

**Fix:** Either drop the inner `try/catch` and rely on the top-level handler
(keeping only the install hint as a `console.error` before re-throwing), or
suppress the generic outer message when the inner one has already printed.
Low-priority polish.

---

_Reviewed: 2026-04-15T14:15:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
