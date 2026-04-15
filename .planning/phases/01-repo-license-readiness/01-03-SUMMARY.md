---
phase: 01-repo-license-readiness
plan: 03
subsystem: docs
tags:
  - licensing
  - compliance
  - manual
  - gap-closure
requires:
  - 01-01
provides:
  - User manual source stripped of legal notices section
  - Regenerated PDF + DOCX artifacts on disk matching stripped source
  - Gap G-01 closure (see 01-VERIFICATION.md)
affects:
  - docs/generate_manual.js
  - docs/OpenSpatialDelay_Manual_v1.0.docx
  - docs/OpenSpatialDelay_Manual_v1.0.pdf
tech_stack:
  added: []
  patterns:
    - DOCX -> PDF via LibreOffice headless (soffice --convert-to pdf) since docx package only emits DOCX
key_files:
  created: []
  modified:
    - docs/generate_manual.js
    - docs/OpenSpatialDelay_Manual_v1.0.docx
    - docs/OpenSpatialDelay_Manual_v1.0.pdf
decisions:
  - Removed stale TOC entry "Third-Party Notices" pointing to dropped "legal-notices" bookmark (Rule 1 auto-fix, directly caused by T1 edit)
  - Left function buildLegalNotices() definition intact in source (dead code, out of scope per plan)
metrics:
  duration_minutes: ~5
  tasks_completed: 2
  completed_date: 2026-04-15
requirements:
  - REPO-02
---

# Phase 01 Plan 03: Remove Legal Notices from User Manual Summary

One-liner: Stripped the `...buildLegalNotices()` spread (and a stale TOC entry) from `docs/generate_manual.js`, regenerated DOCX via Node + PDF via LibreOffice headless, and verified both binary artifacts no longer contain third-party or project-license text — closing gap G-01.

## Tasks Executed

### Task 01-03-T1 — Remove legal notices from assembly array
- **Commit:** 517a39b
- **Change:** Removed two consecutive lines from the assembly array in `docs/generate_manual.js`:
  - Pre-edit line 1689: `    new Paragraph({ children: [new PageBreak()] }),`
  - Pre-edit line 1690: `    ...buildLegalNotices(),`
- **Anchor:** The block now reads `...buildTroubleshooting(), new Paragraph({ children: [new PageBreak()] }), ...buildBackPage(),`
- **Function definition** `function buildLegalNotices()` at line 1414 intentionally left intact (dead code, out of scope).
- **Verification:**
  - `grep -c "\.\.\.buildLegalNotices()" docs/generate_manual.js` = 0 (OK)
  - `grep -c "function buildLegalNotices" docs/generate_manual.js` = 1 (OK, definition retained)
  - `node -c docs/generate_manual.js` exits 0 (syntax OK)
  - Troubleshooting -> BackPage anchor test passes

### Task 01-03-T2 — Regenerate DOCX and PDF
- **Commit:** df5035e
- **Commands used:**
  1. `NODE_PATH=/opt/homebrew/lib/node_modules node docs/generate_manual.js`
     (docx package is not installed in the worktree's node_modules; it is available globally at `/opt/homebrew/lib/node_modules/docx@9.6.1`. NODE_PATH was used to resolve the require. This does not modify the source.)
  2. `soffice --headless --convert-to pdf docs/OpenSpatialDelay_Manual_v1.0.docx --outdir docs/`
- **Deviation (Rule 1 auto-fix — bug directly caused by T1 edit):**
  - After regeneration, `pdftotext` still showed "11 Third-Party Notices" in the TOC because `docs/generate_manual.js` line 520 contained a hand-written `tocEntry("Third-Party Notices", "legal-notices")`. That TOC entry now pointed to a non-existent bookmark (the `legal-notices` heading was removed with the section), making it both stale and a broken internal link.
  - Fix: removed the TOC entry, regenerated DOCX and PDF.
- **Pre/post file sizes:**
  | Artifact | Pre (bytes) | Post (bytes) | Δ         |
  | -------- | ----------- | ------------ | --------- |
  | DOCX     | 1,310,487   | 1,306,895    | -3,592    |
  | PDF      | 784,189     | 786,219      | +2,030    |
  Note: The DOCX is smaller as expected (legal section plus TOC entry removed). The PDF grew slightly because LibreOffice's rendering (font embedding, page layout) differs from whatever tool generated the previous PDF — within expected variance for a different renderer. The critical test is textual content, which is verified empty below.
- **Content verification:**
  - DOCX: `unzip -p ... word/document.xml | grep -i "third-party notices\|project license"` returns empty (OK)
  - PDF:  `pdftotext ... - | grep -i "third-party\|project license"` returns empty (OK)
  - Both artifacts have mtime newer than `docs/generate_manual.js` (OK)

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Stale TOC entry broke after section removal**
- **Found during:** Task 01-03-T2 (post-regeneration verification)
- **Issue:** `docs/generate_manual.js:520` held a hand-written `tocEntry("Third-Party Notices", "legal-notices")` whose bookmark target was removed in T1. Regenerated PDF still showed "11 Third-Party Notices" in the TOC as a broken link.
- **Fix:** Removed the TOC entry, re-ran node generator + soffice PDF conversion.
- **Files modified:** `docs/generate_manual.js`
- **Commit:** df5035e (folded into T2 since the regeneration step was the one that exposed it)

### Environmental Notes

- The `docx` node package is installed globally (`/opt/homebrew/lib/node_modules/docx`), not locally in this worktree. Used `NODE_PATH` to resolve. No `package.json` exists for docs; a future plan may add one for reproducibility.
- LibreOffice (`soffice`) emitted a harmless `--localstorage-file` warning during conversion; output was correct.

## Known Stubs

None — the manual content itself is unchanged (just the legal section removed). The `buildLegalNotices()` function definition is deliberately retained as orphaned code per the plan's scope note (dead code cleanup is out of scope for this gap).

## Self-Check: PASSED

**Files exist:**
- FOUND: docs/generate_manual.js (modified)
- FOUND: docs/OpenSpatialDelay_Manual_v1.0.docx (regenerated, 1,306,895 bytes)
- FOUND: docs/OpenSpatialDelay_Manual_v1.0.pdf (regenerated, 786,219 bytes)
- FOUND: .planning/phases/01-repo-license-readiness/01-03-SUMMARY.md (this file)

**Commits exist:**
- FOUND: 517a39b fix(01-03): remove legal notices section from user manual assembly
- FOUND: df5035e fix(01-03): regenerate user manual without legal notices section

**Success criteria for gap G-01:**
- [x] User manual source no longer assembles the legal section
- [x] Binary artifacts on disk (PDF + DOCX) contain no "Third-Party Notices" or "Project License" text
- [x] `buildLegalNotices()` function definition is left intact (dead code, out of scope)
- [x] Source syntax parses (`node -c`)
