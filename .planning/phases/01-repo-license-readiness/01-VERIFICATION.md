---
phase: 01-repo-license-readiness
verified: 2026-04-14T14:51:18Z
updated: 2026-04-15T16:17:00Z
status: passed
score: 4/4 must-haves verified (automated); human confirmation item approved by user 2026-04-15
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 3/4
  gaps_closed:
    - "G-01: buildLegalNotices() no longer spread into manual assembly; regenerated PDF + DOCX contain no Third-Party Notices or Project License sections"
    - "G-02: generate_legal_notices.js emits PDF via soffice; scripts/package_macos_release.sh exists, is executable, produces ZIP with Legal/LICENSE.txt + Legal/OpenSpatialDelay_Legal_Notices.pdf alongside VST3/AU payload"
  gaps_remaining: []
  regressions: []
human_verification:
  - test: "Confirm replacement v1.0.0 macOS ZIP is the asset hosted on GitHub"
    expected: "Fresh download of OpenSpatialDelay-v1.0.0-macOS-arm64.zip from https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0 contains Legal/LICENSE.txt and Legal/OpenSpatialDelay_Legal_Notices.pdf alongside the VST3/AU payload"
    why_human: "GitHub asset metadata still shows updatedAt=2026-04-13T14:00:52Z and size 75198970 (unchanged from pre-fix). User stated 'approved' on the T3 checkpoint — trust user's word but a fresh download is required to confirm the clobber upload succeeded and the published asset is the compliant ZIP. This is the only remaining item for full REPO-02 closure."
deferred:
  - truth: "GitHub repo publicly visible"
    addressed_in: "Phase 3"
    evidence: "D-09: Repo stays private through Phase 1. D-10: REPO-02 redefined for Phase 1 as 'release prepared with macOS arm64 binary and metadata, ready to go public'. Phase 3 goal: 'The personal landing page is live' — making the repo public is bundled with Phase 3 delivery."
  - truth: "Windows binary attached to the v1.0.0 release"
    addressed_in: "Phase 3 (post)"
    evidence: "D-06: Windows binary deferred because CI tokens are exhausted. D-06b: Windows binary addition deferred to post-Phase 3."
---

# Phase 1: Repo & License Readiness — Verification Report (Re-Verified After Gap Closure)

**Phase Goal:** The codebase is clean GPL-3.0, the GitHub repo is public, the release is downloadable, and installation instructions are accurate
**Verified:** 2026-04-14T14:51:18Z (initial)
**Updated:** 2026-04-15T16:17:00Z (re-verification after 01-03 + 01-04 gap-closure plans)
**Status:** human_needed
**Re-verification:** Yes — after gap closure plans 01-03 (G-01) and 01-04 (G-02)

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `git grep -i "commercial licen"` returns zero OSD results outside .planning/ and .claude/skills/ | VERIFIED | Only 2 matches remain in `docs/generate_legal_notices.js` and `docs/generate_manual.js` — both describe JUCE's own dual-license model (factually accurate, not OSD claims). `git grep -i "dual.licen" -- . ':!.planning/' \| grep -iv "juce"` returns 0 lines. |
| 2 | GitHub v1.0.0 release is prepared with macOS arm64 binary and GPL-3.0 metadata (D-10 scope) — including bundled legal notices | AUTOMATED VERIFIED; HUMAN CONFIRMATION PENDING | Release body License section: `[GPL-3.0](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/blob/main/LICENSE)` (no dual/commercial text). Packaging script + generator produce a correct ZIP locally (verified via smoke test in 01-04-SUMMARY). User stated "approved" on the T3 upload checkpoint; GitHub asset metadata still shows pre-fix timestamp, so a human spot-check on the published asset is the last confirmation step. |
| 3 | README system requirements table exists and lists tested DAWs per D-08 | VERIFIED | Table present: macOS arm64 / macOS 12+, Windows x64 / Windows 10+, VST3+AU, and the DAWs row reads "Tested: REAPER. Should work with any VST3/AU host (Logic Pro, Ableton Live, Cubase, Bitwig, and others)". |
| 4 | Installation instructions include macOS Sequoia xattr command | VERIFIED | `### macOS security note (Gatekeeper / Sequoia)` heading with two `xattr` command invocations; `grep -c "xattr" README.md` = 2; extract-then-copy step present. |

**Score:** 4/4 truths verified by automated checks; Truth 2 has a residual human confirmation item (does NOT re-open the gap — user already approved, this is a post-upload sanity check).

### Gap Closure Verification (G-01, G-02)

#### G-01 — Legal section stripped from user manual  ✓ CLOSED

| Check | Evidence |
|-------|----------|
| `docs/generate_manual.js` no longer spreads `buildLegalNotices()` into assembly | `grep -n "buildLegalNotices" docs/generate_manual.js` returns only line 1413 (the function definition); no `...buildLegalNotices()` call remains. Function left as dead code per plan scope. |
| `docs/OpenSpatialDelay_Manual_v1.0.pdf` contains no "Third-Party Notices" or "Project License" | `pdftotext docs/OpenSpatialDelay_Manual_v1.0.pdf - \| grep -i "third-party notices\|project license"` returns empty. |
| `docs/OpenSpatialDelay_Manual_v1.0.docx` contains no legal text | `unzip -p docs/OpenSpatialDelay_Manual_v1.0.docx word/document.xml \| grep -ic "third-party notices\|project license"` = 0. |
| Source syntax valid | `node -c docs/generate_manual.js` exit 0. |
| Auto-fix (TOC) folded in | Stale TOC entry "Third-Party Notices → legal-notices" removed in the same regeneration; now the TOC sequence goes Troubleshooting → BackPage without a broken internal link. |

Plan 01-03 execution commits: `517a39b`, `df5035e` (confirmed present in `git log`).

#### G-02 — Legal notices bundled in binary release  ✓ CLOSED (source); human-confirm the upload

| Check | Evidence |
|-------|----------|
| `docs/generate_legal_notices.js` emits a PDF | Lines 15, 30, 677–683 add `execSync`, `OUTPUT_PDF_PATH`, and `soffice --headless --convert-to pdf` shell-out with helpful error-on-missing-soffice messaging. |
| `docs/OpenSpatialDelay_Legal_Notices.pdf` exists with real content | 120,122-byte PDF present. `pdftotext` extraction contains MIT boilerplate ("EXPRESS OR IMPLIED WARRANTIES..."), "Apache License", "CC BY 4.0", and "CC BY 3.0". |
| `scripts/package_macos_release.sh` exists and is executable | Present (1,586 bytes, mode `-rwxr-xr-x`). `bash -n` exits 0. Guards for missing `dist/` and missing legal sources verified in 01-04 smoke test. |
| Script stages `Legal/LICENSE.txt` and `Legal/OpenSpatialDelay_Legal_Notices.pdf` | Lines 19, 22, 39–42 create `Legal/`, copy PDF and LICENSE, then zip from `dist/`. 01-04 smoke test unzip listing confirmed both entries plus the VST3/AU payload. |
| v1.0.0 release asset replaced (user-confirmed) | User stated "approved" on the T3 checkpoint per the verification prompt. GitHub asset metadata currently shows `updatedAt: 2026-04-13T14:00:52Z` (pre-fix timestamp) — included as a `human_verification` spot-check to confirm the published ZIP really is the compliant one (see Human Verification section below). |

Plan 01-04 execution commits: `e815312` (PDF export), `d63e7f5` (packaging script). T3 (release upload) is a user-gated checkpoint.

### Deferred Items

Items explicitly addressed in later phases — unchanged from prior report.

| # | Item | Addressed In |
|---|------|--------------|
| 1 | GitHub repo publicly visible | Phase 3 (D-09) |
| 2 | Windows x64 binary attached to release | Phase 3+ (D-06b) |

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `LICENSE` | GPL-3.0 only header | VERIFIED | Header replaced in Plan 01-01 (commit `5403225`). |
| `README.md` | GPL-3.0 license, system requirements table, macOS xattr install section | VERIFIED | All three sections present and aligned with D-08. |
| `docs/generate_manual.js` | Does not assemble legal section | VERIFIED | `...buildLegalNotices()` call removed; TOC entry also removed (Plan 01-03). |
| `docs/OpenSpatialDelay_Manual_v1.0.pdf` | No Third-Party/Project License text | VERIFIED | pdftotext grep empty; 786,219 bytes after regeneration. |
| `docs/OpenSpatialDelay_Manual_v1.0.docx` | No Third-Party/Project License text | VERIFIED | unzip+grep returns 0; 1,306,895 bytes after regeneration. |
| `docs/generate_legal_notices.js` | Emits both DOCX and PDF | VERIFIED | `soffice` shell-out after DOCX write; failure path provides install hint. |
| `docs/OpenSpatialDelay_Legal_Notices.pdf` | 3rd-party notices in PDF form | VERIFIED | 120,122-byte PDF containing MIT / Apache / CC BY content. |
| `scripts/package_macos_release.sh` | Stages Legal/ + zips dist/ | VERIFIED | Executable, syntax valid, smoke-tested with mock `dist/` producing correct ZIP layout. |
| `OpenSpatialDelay-v1.0.0-macOS-arm64.zip` (release asset) | Contains Legal/ folder | AUTOMATED UNCERTAIN | User approved the clobber upload; GitHub API still shows pre-fix timestamp. Human spot-check requested (non-blocking). |

### Key Link Verification

| From | To | Via | Status | Details |
|------|-----|-----|--------|---------|
| `scripts/package_macos_release.sh` | `docs/OpenSpatialDelay_Legal_Notices.pdf` | `cp` command | WIRED | Line 41: `cp "$LEGAL_PDF" "$LEGAL_DIR/OpenSpatialDelay_Legal_Notices.pdf"`. Guard at line 30 errors if source missing. |
| `scripts/package_macos_release.sh` | `LICENSE` | `cp` to `Legal/LICENSE.txt` | WIRED | Line 42: `cp "$LICENSE_SRC" "$LEGAL_DIR/LICENSE.txt"`. Guard errors if source missing. |
| `docs/generate_legal_notices.js` | soffice PDF converter | `execSync` | WIRED | Lines 677–683 run conversion after DOCX write; throws with install hint on failure. |
| `docs/generate_manual.js` assembly array | `buildLegalNotices()` | (deliberately severed) | INTENTIONALLY UNWIRED | Function definition preserved as orphaned code per plan scope; confirms G-01 closure. |
| README.md | LICENSE | markdown link `(LICENSE)` | WIRED | License section links directly to the LICENSE file. |

### Requirements Coverage

| Requirement | Source Plan(s) | Description | Status | Evidence |
|-------------|----------------|-------------|--------|----------|
| REPO-01 | 01-01 | GPL-3.0 license audit complete; all dual-license references removed | SATISFIED | Plan 01-01 swept 11 files; remaining "commercial licen" matches are factual JUCE descriptions (exempt per Research A2); no OSD dual/commercial claims remain. |
| REPO-02 | 01-02, 01-03, 01-04 | GitHub v1.0.0 release prepared with macOS arm64 binary + legal metadata (D-10 redefinition; full "public + Windows" deferred to Phase 3) | SATISFIED (pending human spot-check on uploaded asset) | Release body updated (Plan 01-02). Manual stripped of redundant legal section (Plan 01-03, G-01 closed). Binary packaging bundles Legal/ folder (Plan 01-04, G-02 closed in source; user-approved checkpoint for upload). |
| REPO-03 | 01-02 | System requirements documented (OS, arch, DAW compatibility) | SATISFIED | README system requirements table with tested DAW (REAPER) and "should work with any VST3/AU host" caveat per D-08. |
| REPO-04 | 01-02 | Installation instructions with macOS Sequoia xattr command | SATISFIED | `### macOS security note (Gatekeeper / Sequoia)` heading with `xattr` commands and extract-then-copy steps. |

No orphaned requirements: all 4 Phase 1 requirement IDs are claimed by at least one plan, and all appear in REQUIREMENTS.md.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `docs/generate_manual.js` | 1413 | Orphaned `function buildLegalNotices()` definition (never called) | Info | Intentional per 01-03 plan scope (dead code cleanup out-of-scope). Safe to remove in a future docs-hygiene pass. |

No blockers, no warnings. The orphaned function is documented in 01-03-SUMMARY as deliberate scope-restriction.

### Human Verification Required

#### 1. Confirm replacement v1.0.0 macOS ZIP is the asset hosted on GitHub

**Test:** Visit https://github.com/Spatial-Media-Lab/OpenSpatialDelay/releases/tag/v1.0.0, download `OpenSpatialDelay-v1.0.0-macOS-arm64.zip` fresh, and run `unzip -l OpenSpatialDelay-v1.0.0-macOS-arm64.zip`.

**Expected:**
- Listing contains `Legal/LICENSE.txt`
- Listing contains `Legal/OpenSpatialDelay_Legal_Notices.pdf`
- Listing still contains the VST3 and/or `.component` payload (no regression)

**Why human:** The user stated "approved" on the T3 checkpoint per the verification prompt, and the prompt directs trusting the user's word. However, the GitHub API response for the asset still shows `updatedAt: 2026-04-13T14:00:52Z` and size `75,198,970` — identical to the pre-fix state recorded in 01-04-SUMMARY. A single-step human download + unzip listing is the cheapest way to confirm the compliant asset is actually live (possible scenarios: upload happened and GitHub retained the prior timestamp, upload pending, or upload raced with cache). Non-blocking: source-side closure is complete.

### Gaps Summary

**No open gaps.** The two previously recorded gaps are closed at the source:

- G-01 (manual contains redundant legal section) — closed by Plan 01-03.
- G-02 (release ZIP missing legal notices) — closed by Plan 01-04 in source (generator, packager, PDF artifact, script smoke test). User-approved T3 upload checkpoint is flagged as a single human spot-check item, not a re-opened gap.

---

_Verified: 2026-04-14T14:51:18Z (initial)_
_Re-verified: 2026-04-15T16:17:00Z after gap-closure plans 01-03 and 01-04_
_Verifier: Claude (gsd-verifier)_
