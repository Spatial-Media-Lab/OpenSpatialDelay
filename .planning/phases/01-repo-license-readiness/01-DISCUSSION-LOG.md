# Phase 1: Repo & License Readiness - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-04-14
**Phase:** 01-repo-license-readiness
**Areas discussed:** License transition scope, Release assets & platforms, System requirements format, Repo publicity timing

---

## License Transition Scope

| Option | Description | Selected |
|--------|-------------|----------|
| All 10 files (Recommended) | Clean sweep — every file that says 'dual' or 'commercial license' gets updated to GPL-3.0 only | ✓ |
| Public-facing only (5 files) | LICENSE, README, CLAUDE.md, manual generator, legal notices generator. Leave SPECIFICATION.md and internal docs as-is | |
| Minimal (LICENSE + README) | Just the two files someone actually reads. Fix the rest later | |

**User's choice:** All 10 files
**Notes:** Success criterion (`git grep -i "commercial licen"` returns zero) requires this level of thoroughness anyway.

### Follow-up: Manual PDF Regeneration

| Option | Description | Selected |
|--------|-------------|----------|
| Yes, regenerate (Recommended) | Run generate_manual.js to produce updated PDF | |
| No, I'll handle the PDF | Update .js source only, user regenerates PDF manually | ✓ |

**User's choice:** No, I'll handle the PDF
**Notes:** User will regenerate the manual PDF separately after the generator scripts are updated.

---

## Release Assets & Platforms

| Option | Description | Selected |
|--------|-------------|----------|
| macOS arm64 only | Ship what exists | |
| macOS arm64 + Windows x64 | Cover two main DAW platforms | ✓ |
| macOS arm64 + x86_64 + Windows x64 | Full coverage including Intel Mac | |
| You decide | Claude picks based on CI capabilities | |

**User's choice:** macOS arm64 + Windows x64
**Notes:** Intel Mac users use Rosetta 2. No separate x86_64 build needed.

### Follow-up: Windows Build Method

| Option | Description | Selected |
|--------|-------------|----------|
| Trigger CI and upload artifact (Recommended) | Use existing build-windows.yml workflow | ✓ |
| I'll build it myself | User has Windows build environment | |

**User's choice:** Trigger CI and upload artifact
**Notes:** build-windows.yml workflow already exists in .github/workflows/.

---

## System Requirements Format

| Option | Description | Selected |
|--------|-------------|----------|
| README section (Recommended) | Add System Requirements section to README.md | ✓ |
| Separate SYSTEM_REQUIREMENTS.md | Dedicated file linked from README | |
| Both README + docs/ | Short summary in README, detailed table in docs/ | |

**User's choice:** README section
**Notes:** Single source of truth, visible on GitHub landing page.

### Follow-up: DAW Compatibility Detail

| Option | Description | Selected |
|--------|-------------|----------|
| Tested DAWs only | List only personally verified DAWs | |
| Tested + expected compatible | Tested DAWs explicitly, plus 'should work with any VST3/AU host' note | ✓ |
| You decide | Claude picks standard format | |

**User's choice:** Tested + expected compatible
**Notes:** Honest about what's tested, but sets broader expectations without over-promising.

---

## Repo Publicity Timing

| Option | Description | Selected |
|--------|-------------|----------|
| End of Phase 1 (Recommended) | Make public as last step of Phase 1 | |
| After Phase 2 | Wait until email capture + Patreon ready | |
| After Phase 3 (website live) | Maximum polish — website, email, Patreon all ready | ✓ |

**User's choice:** After Phase 3
**Notes:** User wants full landing page infrastructure before anyone discovers the repo.

### Follow-up: Phase 1 REPO-02 Scope

| Option | Description | Selected |
|--------|-------------|----------|
| Prepare everything, flip later | Phase 1 does all prep, visibility flip is a single action post-Phase 3 | |
| Move 'make public' to Phase 3 | Formally move the visibility change to Phase 3 scope | ✓ |

**User's choice:** Move 'make public' to Phase 3
**Notes:** REPO-02 redefined for Phase 1 as "release prepared with correct binaries and metadata." Publicly visible is a Phase 3 deliverable.

---

## Claude's Discretion

- Order of file updates during license cleanup
- Exact GPL-3.0 license statement wording (as long as accurate and consistent)
- System requirements table format and layout

## Deferred Ideas

- Manual PDF regeneration (user handles)
- macOS x86_64 (Intel) binary (Rosetta 2 sufficient for now)
- Make repo public (moved to Phase 3)
