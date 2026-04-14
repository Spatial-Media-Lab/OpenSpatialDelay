---
phase: 01-repo-license-readiness
plan: "02"
subsystem: documentation
tags: [readme, release, system-requirements, installation, gpl-3.0]
dependency_graph:
  requires: [01-01]
  provides: [REPO-02, REPO-03, REPO-04]
  affects: [README.md, GitHub-release-v1.0.0]
tech_stack:
  added: []
  patterns: []
key_files:
  created: []
  modified:
    - README.md
decisions:
  - "DAWs row uses 'Tested: REAPER' with 'should work with any VST3/AU host' caveat per D-08 — no promises for untested hosts"
  - "macOS security note heading now includes (Gatekeeper / Sequoia) for clarity"
  - "Installation step explicitly mentions extracting ZIP before copying plugin bundles"
metrics:
  duration: "~15 minutes"
  completed: "2026-04-14"
  tasks_completed: 2
  files_modified: 1
---

# Phase 01 Plan 02: Release Body and README Requirements Summary

**One-liner:** Updated GitHub v1.0.0 release body from dual-license to GPL-3.0 link, and refined README system requirements and installation instructions per D-08.

## What Was Built

Two documentation updates:

1. **GitHub v1.0.0 release body (API resource):** The `### License` section previously read `Dual-licensed: GPL-3.0 / Commercial (Spatial Media Lab)`. Updated via `gh release edit --notes-file` to `[GPL-3.0](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/blob/main/LICENSE)`. All other release content and both assets (`OpenSpatialDelay-v1.0.0-macOS-arm64.zip`, `OpenSpatialDelay-v1.0.0-source.zip`) remain unchanged.

2. **README.md (3 edits):**
   - System requirements DAWs row: changed from "Any VST3/AU host — tested in Reaper; expected to work in..." to "Tested: REAPER. Should work with any VST3/AU host (Logic Pro, Ableton Live, Cubase, Bitwig, and others)" — per D-08 (list tested DAWs explicitly, use "should work" caveat for untested)
   - macOS security note heading: added "(Gatekeeper / Sequoia)" parenthetical for clarity
   - macOS installation step: added explicit "Download the ZIP, extract it, then copy..." wording — previously jumped directly to copy without mentioning extraction

## Tasks Completed

| Task | Name | Commit | Files |
|------|------|--------|-------|
| 1 | Update GitHub v1.0.0 release body | (GitHub API — no git commit) | GitHub release body |
| 2 | Review and expand README system requirements and installation | 228427f | README.md |

## Verification Results

- `gh release view v1.0.0 --json body` — Matches: 0 for "commercial" or "dual"; body contains `[GPL-3.0](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/blob/main/LICENSE)`; assets unchanged (2 ZIPs)
- `grep -A8 "## System requirements" README.md` — table present with macOS arm64, Windows x64, VST3/AU, and explicit REAPER tested row
- `grep -c "xattr" README.md` — 2 (xattr commands present)
- `grep "macOS security note" README.md` — heading contains "(Gatekeeper / Sequoia)"
- `grep "extract" README.md` — extract step present in macOS installation instructions

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Restored GPL-3.0 License section in README (worktree state regression)**
- **Found during:** Task 2 commit preparation
- **Issue:** `git reset --soft` to the base commit left the working tree with the old branch's README, which had the dual-license `## License` section. The base commit already had GPL-3.0 only text (from Plan 01-01's merge). Without fixing this, the commit would have re-introduced the dual-license block.
- **Fix:** Restored the `## License` section in the working tree to match the base commit's GPL-3.0 only text before committing. The commit diff ends up containing only the 3 intended Plan 01-02 changes.
- **Files modified:** README.md
- **Commit:** 228427f (same commit as Task 2 — the fix was part of making the commit clean)

**2. [Rule 1 - Bug] Unstaged unwanted staged files from git reset --soft**
- **Found during:** Task 2 commit
- **Issue:** `git reset --soft` staged deletions of all `.planning/` files and modifications to CLAUDE.md, LICENSE, SPECIFICATION.md, and other files from the old branch commits — these were not part of Plan 01-02's scope.
- **Fix:** Used `git restore --staged` to unstage all files except README.md before committing.
- **Files modified:** None (index cleanup only)
- **Commit:** N/A

## Known Stubs

None. All changes are substantive documentation updates.

## Threat Flags

None. Documentation-only changes; no new attack surface introduced. The `gh release edit` call was authenticated via existing `gh` CLI session.

## Self-Check: PASSED

- GitHub v1.0.0 release body updated: confirmed — "Matches: 0" for commercial/dual, GPL-3.0 link present
- GitHub release assets unchanged: confirmed — both ZIPs still attached
- README.md system requirements table: confirmed — macOS arm64, Windows x64, VST3/AU, tested REAPER
- README.md xattr commands present: confirmed — count 2
- README.md macOS security heading updated: confirmed — "(Gatekeeper / Sequoia)" present
- README.md extract ZIP step added: confirmed — "extract it" in macOS install line
- Commit 228427f exists and contains only README.md (1 file, 6 lines changed): confirmed
- No dual-license regression in committed README: confirmed via `git diff HEAD^ HEAD README.md`
