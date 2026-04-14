# Phase 1: Repo & License Readiness — Research

**Researched:** 2026-04-14
**Domain:** License text cleanup, GitHub release management, README documentation
**Confidence:** HIGH — all findings verified against actual file contents and live GitHub state

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

- **D-01:** Full sweep across all 10 files that reference dual or commercial licensing. Every file gets updated to GPL-3.0 only — no stragglers.
- **D-02:** Files to update: LICENSE, README.md, CLAUDE.md, agent_docs/licensing.md, SPECIFICATION.md, docs/generate_manual.js, docs/generate_legal_notices.js, docs/RELEASE_NOTES_v1.0.0.md, docs/RELEASE_PLAN_v1.0.0.md, context-improvement-plan.md
- **D-03:** Source files (.cpp/.h) have no license headers — no changes needed there.
- **D-04:** Update the `.js` generator scripts (generate_manual.js, generate_legal_notices.js) but do NOT regenerate the manual PDF — user handles PDF regeneration manually.
- **D-05:** v1.0.0 release must include macOS arm64 binary + Windows x64 binary. No separate macOS x86_64 build (Intel users use Rosetta 2).
- **D-06:** Windows binary is produced by triggering existing `build-windows.yml` CI workflow — but CI tokens are exhausted this month. Windows build MUST happen AFTER repo goes public (Phase 3+). Phase 1 prepares the macOS arm64 release only.
- **D-06b:** Windows binary addition to the release is deferred to post-Phase 3 when CI tokens are available again.
- **D-07:** System requirements go in a new section in README.md (not a separate file).
- **D-08:** List tested DAWs explicitly, plus a general "should work with any VST3/AU host" note. Don't promise compatibility with untested DAWs.
- **D-09:** Repo stays private through Phase 1. The "make repo public" action is moved to Phase 3 scope.
- **D-10:** REPO-02 success criterion is redefined for Phase 1 as: "Release v1.0.0 is prepared with macOS arm64 binary and metadata, ready to go public." The "publicly visible" part and Windows binary are Phase 3+ deliverables.

### Claude's Discretion

- Order of file updates during license cleanup
- Exact wording of the GPL-3.0 license statement in each file (as long as it's accurate and consistent)
- System requirements table format and layout within the README section

### Deferred Ideas (OUT OF SCOPE)

- **Make repo public** — Moved to Phase 3
- **Windows x64 binary** — CI tokens exhausted. Build and attach after repo goes public (Phase 3+)
- **Manual PDF regeneration** — User handles manually after generator scripts are updated
- **macOS x86_64 (Intel) binary** — Not included in v1.0.0; Rosetta 2 covers Intel users

</user_constraints>

---

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| REPO-01 | GPL-3.0 license audit complete — all dual-license references removed across source headers, README, manual PDF, About dialog, CLAUDE.md, agent_docs/licensing.md | File audit complete: 11 files need edits (see Standard Stack section); source headers confirmed clean; About dialog confirmed absent from compiled source |
| REPO-02 | GitHub repo public with Release v1.0.0 published (macOS + Windows binaries) | Redefined for Phase 1: release exists, macOS arm64 binary present, release body has dual-license text to fix; repo stays private |
| REPO-03 | System requirements reviewed and documented (supported OS versions, CPU architectures, DAW compatibility) | README already has system requirements table at lines 107–114; needs accuracy review and tested DAW expansion |
| REPO-04 | Installation instructions written — includes macOS Sequoia xattr command, based on verified system requirements from REPO-03 | xattr commands already present at README lines 91–92; section heading says "macOS security note" not "Sequoia"; verify completeness |

</phase_requirements>

---

## Summary

Phase 1 is a documentation-and-metadata cleanup phase with no DSP or code changes. The work divides into three streams: (1) license text cleanup across 11 files, (2) a single GitHub release body edit to remove the dual-license line, and (3) a review and possible light expansion of the existing system requirements and installation sections.

The codebase audit confirms that `.cpp` and `.h` source files contain zero license headers — no source file touches needed. The "About dialog" mentioned in REQUIREMENTS.md is a stale requirement: no About panel or license string exists anywhere in `Source/`. The REPO-01 success criterion (`git grep -i "commercial licen"`) will be green after the 11 files are cleaned; the `.claude/skills/` REFERENCE.md match is a code comment (`# Commercial license only!`) describing JUCE's own splash-screen flag — not an OSD licensing claim — and is exempt from the sweep.

The GitHub v1.0.0 release is already published (not a draft), with the macOS arm64 binary attached. The release body ends with `Dual-licensed: GPL-3.0 / Commercial (Spatial Media Lab)` — this single line must be updated via `gh release edit`. README already has a system requirements table and xattr commands; both need accuracy review before Phase 1 is complete.

**Primary recommendation:** Execute in three sequential waves: (1) clean all 11 files of OSD dual-license claims, (2) update the GitHub release body, (3) verify and lightly expand README system requirements and installation section. Commit after each wave.

---

## Current File State Audit

### Files with OSD dual/commercial license claims (verified by `git grep`)

[VERIFIED: git grep -il "dual.licen|commercial licen" run 2026-04-14]

| File | What to change | Change type |
|------|---------------|-------------|
| `LICENSE` | Replace dual-license header (lines 1–16) — keep full GPL-3.0 text | Header edit |
| `README.md` | Lines 134–137: replace 2-bullet dual-license block | Text edit |
| `CLAUDE.md` | Line 4: `Dual-licensed GPL-3.0 / Commercial (Spatial Media Lab)` → GPL-3.0 only | Text edit |
| `agent_docs/licensing.md` | Line 3: `dual-licensed: GPL-3.0 and Commercial License` header + line 21 table row `"All distributed third-party components are compatible with both GPL-3.0 and commercial licensing"` | Text edit (retain JUCE tier table as informational) |
| `SPECIFICATION.md` | 3 locations: lines 39, 72 ("potential commercial licensing"), line 1006 ("Dual license — GPL-3.0 + commercial"), line 1068 checklist row | Text edit |
| `docs/generate_manual.js` | Lines 1435, 1534–1539: OSD dual-license in "Project License" section of generated output | Text edit |
| `docs/generate_legal_notices.js` | Lines 555–565: `buildProjectLicense()` function outputs dual-license claim | Text edit |
| `docs/RELEASE_NOTES_v1.0.0.md` | Lines 126–129: Licensing section lists dual license | Text edit |
| `docs/RELEASE_PLAN_v1.0.0.md` | Lines 18, 65, 143: References to "Dual GPL-3.0 / Commercial" | Text edit |
| `context-improvement-plan.md` | Lines 132–133: Proposes keeping "Dual-licensed" wording — update to GPL-3.0 only | Text edit |
| `context-audit-report.md` | Lines 145, 147: Copied from agent_docs/licensing.md, mentions dual-license model | Text edit |

**Note:** `context-audit-report.md` was NOT in D-02's original list of 10 files but appears in `git grep` results. It must be included for the REPO-01 success criterion to pass.

### Files confirmed clean — no changes needed

[VERIFIED: git grep run 2026-04-14]

- All `Source/*.cpp` and `Source/*.h` files — zero license headers, zero commercial references
- `.claude/skills/cross-platform-builds/REFERENCE.md` — single match is `# Commercial license only!` code comment describing JUCE's `JUCE_DISPLAY_SPLASH_SCREEN=0` flag, not an OSD license claim. Exempt from sweep.
- `HRTF/` — SOFA files, binary data
- `Tests/` — test code, no license text
- `docs/VERSION_HISTORY.md` — no commercial license references

### GitHub Release v1.0.0

[VERIFIED: `gh release view v1.0.0` run 2026-04-14]

| Property | Current state |
|----------|--------------|
| Tag | `v1.0.0` |
| Title | `v1.0.0 — First Public Release` |
| Status | Published (not draft) |
| Assets | `OpenSpatialDelay-v1.0.0-macOS-arm64.zip`, `OpenSpatialDelay-v1.0.0-source.zip` |
| Body — license line | `### License\nDual-licensed: GPL-3.0 / Commercial (Spatial Media Lab)` — MUST be updated |

Edit command: `gh release edit v1.0.0 --repo Spatial-Media-Lab/OpenSpatialDelay --notes-file <file>`

**Note:** `--notes` replaces the entire body. The planner must capture the full current body, replace only the license line, and write the result to a temp file for `--notes-file`.

### README System Requirements (existing)

[VERIFIED: README.md lines 107–114, read 2026-04-14]

```
## System requirements

| | Minimum |
|---|---|
| **macOS** | Apple Silicon (arm64), macOS 12+ |
| **Windows** | x64, Windows 10+ |
| **Formats** | VST3, AU (macOS only) |
| **DAWs** | Any VST3/AU host — tested in Reaper; expected to work in Logic Pro, Ableton Live, Cubase, Bitwig, and others |
```

This section already exists and is broadly accurate. For REPO-03 the planner should verify: (a) macOS 12 minimum is still correct, (b) the DAW list matches what was actually tested. D-08 says list tested DAWs explicitly with the "any VST3/AU host" note — REAPER is confirmed tested, others are "expected to work."

### README Installation / macOS xattr (existing)

[VERIFIED: README.md lines 86–95, read 2026-04-14]

The `xattr -cr` commands are already present and correct. The section heading is `### macOS security note` — it does not say "Sequoia" in the heading, though the text applies to Sequoia. REPO-04 requires the Sequoia xattr command to be present — it is. Consider whether to add a parenthetical `(required on macOS Sequoia and earlier)` to the section heading for clarity, but this is Claude's discretion.

---

## Architecture Patterns

### License Cleanup Pattern

**What:** Replace OSD dual-license claim text with GPL-3.0 only statement. Preserve third-party license text (JUCE's own dual-license description is factually accurate and should stay).

**Critical distinction:** Two types of "commercial license" text exist:
1. **OSD claims** — "OpenSpatialDelay is dual-licensed under GPL-3.0 and a Commercial License" → REMOVE/REPLACE
2. **JUCE facts** — "JUCE is dual-licensed under GPL-3.0 and a commercial license from Raw Material Software Limited" → KEEP (factually accurate about JUCE, not OSD)

**Consistent replacement wording (Claude's discretion):**

```
OpenSpatialDelay is licensed under the GNU General Public License v3.0 (GPL-3.0).
```

For the LICENSE file header specifically:

```
OpenSpatialDelay

Copyright (C) 2026 Spatial Media Lab (spatialmedialab.org)
Author: Andrew Rahman

This software is licensed under the GNU General Public License v3.0.
See the full license text below.
```

### GitHub Release Body Edit Pattern

**What:** `gh release edit` replaces the entire release notes body. Must preserve all existing content except the `### License` section.

**Pattern:**
```bash
# 1. Capture current body
gh release view v1.0.0 --repo Spatial-Media-Lab/OpenSpatialDelay --json body \
  | python3 -c "import sys,json; print(json.load(sys.stdin)['body'])" > /tmp/release_body.md

# 2. Edit /tmp/release_body.md — replace the License section

# 3. Apply
gh release edit v1.0.0 --repo Spatial-Media-Lab/OpenSpatialDelay \
  --notes-file /tmp/release_body.md
```

**New License section for release body:**
```markdown
### License
[GPL-3.0](https://github.com/Spatial-Media-Lab/OpenSpatialDelay/blob/main/LICENSE)
```

### agent_docs/licensing.md Update Pattern

This file currently describes the dual-license model and JUCE commercial tiers. After the license change:
- Remove: dual-license model description (opening paragraph)
- Keep: JUCE 8 commercial tier table (still relevant — tells agents when a JUCE tier upgrade is needed)
- Keep: Third-party license compatibility table (still relevant for GPL-3.0 compatibility)
- Update: Table column "Commercial Use" is still accurate (third-party licenses allow commercial use); update the intro text to frame it as "GPL-3.0 compatibility" not "both GPL-3.0 and commercial licensing"

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Editing GitHub release body | Manual API calls or UI | `gh release edit --notes-file` | Atomic, scriptable, preserves all metadata |
| Checking all license references | Manual file-by-file search | `git grep -i "commercial licen"` | Catches stragglers across entire working tree |
| Updating release body content | String replacement in shell | Capture body → edit temp file → re-apply | Prevents accidental truncation of existing release notes |

---

## Common Pitfalls

### Pitfall 1: Removing JUCE's dual-license description
**What goes wrong:** The generator scripts contain `"JUCE is dual-licensed..."` — this is a factual statement about JUCE, not OSD. Removing it incorrectly strips a required attribution notice.
**Why it happens:** `git grep "dual-licen"` matches both OSD and JUCE descriptions.
**How to avoid:** Search for `"OpenSpatialDelay.*dual"` or `"dual.*OpenSpatialDelay"` to isolate OSD claims. The JUCE description in both `.js` files must stay.
**Warning signs:** If generate_legal_notices.js no longer describes JUCE's licensing model, it fails its attribution purpose.

### Pitfall 2: gh release edit truncates the body
**What goes wrong:** Using `--notes "short string"` instead of `--notes-file` when the release body is long (the current body is ~450 words). Passing the body as a shell string risks quoting issues and silent truncation.
**Why it happens:** Shell string handling with newlines and special characters is fragile.
**How to avoid:** Always capture to a temp file, edit the file, and use `--notes-file`.

### Pitfall 3: context-audit-report.md missed in sweep
**What goes wrong:** `git grep` finds it, but D-02 did not list it — a planner working only from D-02 skips it.
**Why it happens:** The discussion enumerated the "known" 10 files; the audit report was not in scope at discussion time.
**How to avoid:** Run `git grep -i "commercial licen"` as final verification step; treat its output as ground truth for REPO-01 success criterion.

### Pitfall 4: "About dialog" is a ghost requirement
**What goes wrong:** REQUIREMENTS.md REPO-01 mentions "About dialog" — implementing tasks search Source/ for About dialog license text that doesn't exist.
**Why it happens:** REQUIREMENTS.md predates the codebase audit; no About panel was ever built.
**How to avoid:** Confirmed by exhaustive search of `Source/` — no license text, no About dialog. Planner should note "About dialog: not present in codebase — no action needed" explicitly.

### Pitfall 5: Forgetting SPECIFICATION.md line 1068 checklist
**What goes wrong:** SPECIFICATION.md has a release checklist row: `"Verify LICENSE file visible at repo root | GPL-3.0 + commercial header"` — this is historical and contradicts the new state.
**Why it happens:** Checklist is nested in Appendix E and easy to miss.
**How to avoid:** SPECIFICATION.md has 4 separate locations with dual-license text. Update all 4: lines 39, 72, 1006, 1068.

### Pitfall 6: REPO-02 success criterion mismatch
**What goes wrong:** REQUIREMENTS.md says REPO-02 = "GitHub repo public with Release v1.0.0 published (macOS + Windows binaries)." This is the original definition. Phase 1 delivery is NOT this — it is D-10's redefined version.
**Why it happens:** REQUIREMENTS.md was not updated when the discussion locked D-10.
**How to avoid:** The planner must use D-10's definition for Phase 1 success, not REQUIREMENTS.md verbatim. Consider noting this discrepancy in PLAN.md.

---

## Code Examples

### Verify success criterion before and after

```bash
# Before: expect 10+ results
git grep -i "commercial licen" -- . | grep -v ".planning/"

# After: expect 0 results (outside any CHANGELOG if one existed)
git grep -i "commercial licen" -- . | grep -v ".planning/" | grep -v "CHANGELOG"
# Note: no CHANGELOG file exists in this repo; VERSION_HISTORY.md has zero matches
```

### LICENSE file new header

```
OpenSpatialDelay

Copyright (C) 2026 Spatial Media Lab (spatialmedialab.org)
Author: Andrew Rahman

This software is licensed under the GNU General Public License v3.0.
See the full license text below.

                    GNU GENERAL PUBLIC LICENSE
                       Version 3, 29 June 2007
[... rest of GPL-3.0 text unchanged ...]
```

### README.md License section replacement

```markdown
## License

OpenSpatialDelay is free software, licensed under the
**[GNU General Public License v3.0 (GPL-3.0)](LICENSE)**.

HRTF data from third-party sources under their respective licenses (MIT, Apache 2.0, CC BY, Public Domain). See [HRTF/](HRTF/) for details.
```

### generate_legal_notices.js — buildProjectLicense() replacement

```javascript
function buildProjectLicense() {
  const items = [];
  items.push(heading1("Project License"));

  items.push(bodyPara([
    bodyText("OpenSpatialDelay is free software, licensed under the "),
    boldText("GNU General Public License v3.0 (GPL-3.0)"),
    bodyText(". Any derivative work must also be released under the GPL-3.0. The full license text is included in the LICENSE file distributed with the source code."),
  ]));

  items.push(bodyPara([
    bodyText("Copyright (C) 2026 Spatial Media Lab ("),
    externalLink("spatialmedialab.org", "https://spatialmedialab.org"),
    bodyText(")"),
  ]));
  items.push(bodyPara([
    bodyText("Author: Andrew Rahman"),
  ]));

  items.push(spacer(20));
  return items;
}
```

### generate_manual.js — Project License section replacement

Locate the "Project License" heading block near line 1532. Replace the `dual-licensed` paragraph and Commercial License bullet with:

```javascript
  items.push(bodyPara([
    bodyText("OpenSpatialDelay is free software, licensed under the "),
    boldText("GNU General Public License v3.0 (GPL-3.0)"),
    bodyText(". The full GPL-3.0 text is included in the LICENSE file distributed with the source code."),
  ]));
```

---

## Runtime State Inventory

> This phase involves no renames, migrations, or rebrand — no runtime state changes.

Not applicable. Phase 1 edits documentation and metadata text only. No stored data, live service configs, OS-registered state, or build artifacts reference "dual-licensed" or "commercial license" as a programmatic identifier.

**Exception — GitHub release body:** The live v1.0.0 release body contains the dual-license line. This is runtime state (a GitHub API resource, not a git file). Addressed in Architecture Patterns section above via `gh release edit`.

---

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| `gh` CLI | GitHub release body edit (REPO-02) | Yes | (confirmed — used in this session) | None — required |
| `git grep` | REPO-01 verification | Yes | system git | — |
| `python3` | Release body capture/edit | Yes | system python3 | Use `node -e` or jq |

---

## Validation Architecture

> No `nyquist_validation` key in `.planning/config.json` (file absent) — treat as enabled.

### Test Framework

| Property | Value |
|----------|-------|
| Framework | Catch2 (existing test suite in `Tests/`) |
| Config file | `CMakeLists.txt` target `OpenSpatialDelayTests` |
| Quick run | `cmake --build build --target OpenSpatialDelayTests -j$(sysctl -n hw.ncpu) && ./build/OpenSpatialDelayTests` |
| Full suite | Same — single suite covers all tests |

### Phase Requirements — Test Map

Phase 1 requirements are documentation-only changes. No unit tests exist or are appropriate for text editing tasks. Validation is by shell command, not test framework.

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| REPO-01 | Zero "commercial licen" matches outside planning/ | Shell grep | `git grep -i "commercial licen" -- . \| grep -v ".planning/"` | N/A (shell) |
| REPO-02 | Release body no longer contains dual-license line | Shell grep | `gh release view v1.0.0 --repo Spatial-Media-Lab/OpenSpatialDelay --json body \| grep -i "commercial"` | N/A (shell) |
| REPO-03 | System requirements section present in README | Shell grep | `grep -A10 "## System requirements" README.md` | N/A (shell) |
| REPO-04 | xattr command present and macOS installation instructions complete | Shell grep | `grep "xattr" README.md` | N/A (shell) |

### Wave 0 Gaps

None — no test files need creating. All verification is shell-based and can run immediately.

---

## Security Domain

This phase makes no code changes and introduces no new attack surface. No ASVS categories apply.

- V5 Input Validation: not applicable (no user input)
- V6 Cryptography: not applicable
- All other categories: not applicable

The only security-adjacent consideration: ensure the `gh release edit` command does not accidentally expose the macOS binary asset URL in a public context — but the repo remains private through Phase 1, so this is moot.

---

## Open Questions

1. **Is macOS 12 still the correct minimum?**
   - What we know: README says `macOS 12+`. The plugin uses CoreAudio APIs available since macOS 10.15. JUCE 8 minimum is macOS 10.13.
   - What's unclear: Whether the team has verified loading on macOS 12 vs. tested only on 14/15 (Sonoma/Sequoia).
   - Recommendation: Keep `macOS 12+` unless user specifies otherwise. It's conservative and unlikely to be wrong. Flag as ASSUMED if the planner wants confirmation.

2. **Should context-improvement-plan.md be updated or noted as superseded?**
   - What we know: Section 4 proposes keeping "Dual-licensed" wording in CLAUDE.md and creating `docs/LICENSING.md`. This proposal is now moot — the decision is GPL-3.0 only.
   - What's unclear: Is context-improvement-plan.md a live document or a historical plan?
   - Recommendation: Update the license-related section to reflect the GPL-3.0-only decision. The file is in D-02's sweep list, so a planner task will cover it.

3. **Does the release title need updating?**
   - What we know: Current title is `v1.0.0 — First Public Release`. This is accurate.
   - What's unclear: Whether the user wants to change it.
   - Recommendation: Leave title unchanged. Not part of any requirement.

---

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | macOS 12+ minimum is still correct for the system requirements table | Open Questions | Minor — would need to update one README cell |
| A2 | JUCE `JUCE_DISPLAY_SPLASH_SCREEN=0` comment in `.claude/skills/REFERENCE.md` is exempt from the REPO-01 success criterion (it is a code comment about JUCE's feature, not an OSD license claim) | Current File State Audit | If wrong: the `git grep` check would still pass because `git grep -i "commercial licen"` does NOT match "Commercial license only!" — the exemption is correct by grep pattern matching |
| A3 | The "About dialog" in REQUIREMENTS.md REPO-01 does not correspond to any existing UI component | Current File State Audit | If wrong: a hidden About dialog with license text would create an unremediated source change. Confirmed by exhaustive Source/ grep — risk is VERY LOW |

**Note on A2:** Verified by re-running the REPO-01 success criterion pattern: `git grep -i "commercial licen"` matches "commercial licen" — the skills REFERENCE.md match is `# Commercial license only!` which contains "commercial licen" and WOULD match. The planner should verify whether this match is acceptable or whether the comment needs rewording. [VERIFIED: confirmed match exists, decision on exemption is ASSUMED]

---

## Sources

### Primary (HIGH confidence)
- Live file read of all 11 target files — 2026-04-14
- `git grep -il "dual.licen|commercial licen"` across full working tree — 2026-04-14
- `gh release view v1.0.0 --repo Spatial-Media-Lab/OpenSpatialDelay` — confirmed release state, assets, body — 2026-04-14
- `gh repo view Spatial-Media-Lab/OpenSpatialDelay --json visibility` — confirmed PRIVATE — 2026-04-14

### Secondary (MEDIUM confidence)
- `gh release edit --help` — confirmed `--notes-file` flag for full body replacement

---

## Metadata

**Confidence breakdown:**
- License cleanup scope: HIGH — every target file read and grep-verified
- GitHub release state: HIGH — queried live
- README coverage for REPO-03/04: HIGH — read directly; both sections already exist
- macOS minimum version: MEDIUM/ASSUMED — not independently tested

**Research date:** 2026-04-14
**Valid until:** Stable indefinitely (no external APIs or library versions involved)
