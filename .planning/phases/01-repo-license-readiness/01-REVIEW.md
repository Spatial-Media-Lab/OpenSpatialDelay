---
phase: 01-repo-license-readiness
reviewed: 2026-04-14T14:45:00Z
depth: standard
files_reviewed: 11
files_reviewed_list:
  - LICENSE
  - README.md
  - CLAUDE.md
  - agent_docs/licensing.md
  - SPECIFICATION.md
  - docs/generate_manual.js
  - docs/generate_legal_notices.js
  - docs/RELEASE_NOTES_v1.0.0.md
  - docs/RELEASE_PLAN_v1.0.0.md
  - context-improvement-plan.md
  - context-audit-report.md
findings:
  critical: 0
  warning: 0
  info: 3
  total: 3
status: issues_found
---

# Phase 1: Code Review Report

**Reviewed:** 2026-04-14T14:45:00Z
**Depth:** standard
**Files Reviewed:** 11
**Status:** issues_found (Info only — no Critical or Warning)

## Summary

Phase 1 is a license transition from OSD dual-license / commercial to GPL-3.0 only across 11 files. The transition is executed cleanly.

Verified outcomes:

- **OSD license text is consistent across all 11 files.** Every OSD license statement now reads "GPL-3.0" (or the full "GNU General Public License v3.0 (GPL-3.0)" expansion). No OSD dual-license or commercial-license wording remains in any reviewed file.
- **JUCE's dual-license descriptions are preserved.** Both JS generators (`generate_manual.js:1435`, `generate_legal_notices.js:293`) correctly still describe JUCE as dual-licensed (GPL-3.0 / commercial from Raw Material Software Limited), which is factually accurate. The table rows listing JUCE 8 as "GPL-3.0 / Commercial" (manual.js:1425, notices.js:282) are also preserved. This matches the context brief: JUCE descriptions SHOULD remain dual-license.
- **JavaScript generators are syntactically valid.** `node --check` passes for both `docs/generate_manual.js` and `docs/generate_legal_notices.js`. Function signatures, `require`s, table structures, and `licenseBlock()` helpers all consistent. No broken edits around the license blocks.
- **README internal link target resolves.** `[macOS security note](#macos-security-note-gatekeeper--sequoia)` at line 65 correctly targets heading "### macOS security note (Gatekeeper / Sequoia)" at line 86. GitHub slugifier rules produce a match (double-hyphen comes from spaces around the `/`).
- **README other linked paths exist:** `LICENSE`, `HRTF/`, `docs/OpenSpatialDelay_Manual_v1.0.pdf`, `docs/VERSION_HISTORY.md`, `docs/assets/screenshot.png`, `docs/assets/sml-logo.png` — all present.
- **README heading hierarchy is flat H2 with H3 sub-sections under Installation only.** No hierarchy violations.
- **LICENSE file header correctly names Spatial Media Lab + Andrew Rahman and drops any "dual-license" preamble.** Body is standard GPL-3.0 v3 text.
- **CLAUDE.md `## Why` line now reads "Licensed under GPL-3.0. See LICENSE for full text."** No stale "Spatial Media Lab" commercial-license hint.
- **`agent_docs/licensing.md` correctly frames OSD as GPL-3.0 while preserving the factual JUCE tier table.**

Findings below are Info-only style/consistency notes. None block the license transition phase.

## Info

### IN-01: libmysofa license disagreement across documentation

**File:** `SPECIFICATION.md:82` and `SPECIFICATION.md:487`
**Issue:** Both lines state `libmysofa (LGPL 2.1+ — compatible with open-source distribution)`. Every other authoritative reference in the repo lists libmysofa v1.3.2 as BSD-3-Clause:

- `agent_docs/licensing.md:26` → `| libmysofa v1.3.2 | BSD-3-Clause | ...`
- `docs/generate_manual.js:1426` → `"libmysofa v1.3.2 (SOFA File Reader)", "BSD-3-Clause", ...`
- `docs/generate_legal_notices.js:283` → `"libmysofa (SOFA File Reader)", "1.3.2", "BSD-3-Clause", ...`
- `context-audit-report.md:166` → `| libmysofa v1.3.2 | BSD-3-Clause | ...`

The BSD-3-Clause designation is consistent with the BSD-3-Clause notice block emitted in `generate_legal_notices.js:314`. libmysofa upstream (github.com/hoene/libmysofa) switched from LGPL to BSD-3-Clause at v1.x. This is a pre-existing discrepancy, not introduced by Phase 1, but it surfaces in a file within Phase 1 review scope. Flagging for visibility.

**Fix:** In `SPECIFICATION.md`, update line 82 and line 487 to state `BSD-3-Clause`. Example:

```diff
- **Spatial audio HRTF parsing:** libmysofa (LGPL 2.1+ — compatible with open-source distribution)
+ **Spatial audio HRTF parsing:** libmysofa (BSD-3-Clause — compatible with GPL-3.0 distribution)
```

and

```diff
- **Parser library:** `libmysofa` (LGPL 2.1+) — lightweight C library for reading SOFA files. Compiles natively on macOS and Windows.
+ **Parser library:** `libmysofa` (BSD-3-Clause) — lightweight C library for reading SOFA files. Compiles natively on macOS and Windows.
```

Optional: defer to a separate follow-up since it's outside the stated Phase 1 scope (license transition only).

### IN-02: Generators not re-run after JS edits — stale DOCX/PDF artifacts

**File:** `docs/generate_manual.js`, `docs/generate_legal_notices.js`, `docs/OpenSpatialDelay_Manual_v1.0.pdf`, `docs/OpenSpatialDelay_Legal_Notices.docx` (output of the generators, not in review scope)
**Issue:** Phase 1 context states "The generators were not re-run as part of this phase." Both JS files were edited (or verified) in Phase 1, but the `.pdf` and `.docx` they produce are binary artifacts that will still reflect any pre-transition text they were last built from. `docs/RELEASE_PLAN_v1.0.0.md:142–143` lists them as "Current", and `docs/RELEASE_PLAN_v1.0.0.md:65–66` specifically says "Verify user manual PDF opens and TOC links work" and "Verify LICENSE file at repo root has GPL-3.0 header" as release gates. If the manual PDF still contains any stale license wording, the release checklist's documentation-consistency claim is broken.

This is informational, not a bug in the reviewed JS source. Generator output verification belongs to a later release-gate phase.

**Fix:** Before v1.0.0 release, run:

```bash
node docs/generate_manual.js
node docs/generate_legal_notices.js
```

and commit the regenerated `.docx` / `.pdf` (or regenerate the PDF via whichever tool converts `.docx` → `.pdf` in the current workflow). Spot-check the "Project License" section and "Third-Party Notices → JUCE" section of each generated document.

### IN-03: `context-audit-report.md` and `context-improvement-plan.md` are historical snapshots, not live docs

**File:** `context-audit-report.md`, `context-improvement-plan.md`
**Issue:** These two files are review-scope per the config but are dated 2026-04-10 audit/plan artifacts. They contain no OSD dual-license or commercial-license claims (only passing `dual` mentions relating to dual-head pitch, dual Notion integrations, etc. — unrelated to licensing) so they required no Phase 1 edit. They do cite the CLAUDE.md licensing section and reference "Licensing tables" that may now be slimmed (per their own recommendations #1 and #4). This is expected for historical planning documents and not a defect.

**Fix:** None required. If freshness matters, add a "Status: historical snapshot (2026-04-10)" banner at the top of both files so future readers do not treat them as current-state references. Purely cosmetic.

---

_Reviewed: 2026-04-14T14:45:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
