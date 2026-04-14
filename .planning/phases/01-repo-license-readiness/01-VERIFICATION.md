---
phase: 01-repo-license-readiness
verified: 2026-04-14T14:51:18Z
updated: 2026-04-14T16:35:00Z
status: gaps_found
score: 3/4 must-haves verified (one material gap found in delivery path)
overrides_applied: 0
gaps:
  - id: G-01
    truth: "Project license section lives in a separate legal notices document, not in the user manual"
    current_state: "`docs/generate_manual.js` line 1690 still calls `buildLegalNotices()`, producing a redundant Third-Party Notices + Project License section inside `OpenSpatialDelay_Manual_v1.0.pdf`. Cross-repo decision was to strip this from the user manual because `docs/generate_legal_notices.js` already produces a dedicated `OpenSpatialDelay_Legal_Notices.docx` covering the same content."
    required_action: "Remove `...buildLegalNotices(),` call (and its preceding page break) from `generate_manual.js` main assembly. Regenerate manual PDF/DOCX so the shipped artifacts no longer contain the redundant legal section."
    files_affected: ["docs/generate_manual.js", "docs/OpenSpatialDelay_Manual_v1.0.pdf", "docs/OpenSpatialDelay_Manual_v1.0.docx"]
    severity: required_for_repo_02
  - id: G-02
    truth: "Legal notices document is bundled with binary release ZIP so GPL-3.0 Section 6 and third-party license notice obligations are satisfied for binary users"
    current_state: "`OpenSpatialDelay_Legal_Notices.docx` exists in `docs/` but is NOT included in `OpenSpatialDelay-v1.0.0-macOS-arm64.zip`. Binary users receive the plugin and nothing else — no LICENSE file, no third-party notices. This is a material compliance gap because third-party licenses (MIT KEMAR, Apache 2.0 Roboto/SADIE, CC BY HUTUBS, SIL OFL DM Sans/JetBrains Mono) require their notices accompany any distribution."
    required_action: "Update release packaging so the macOS arm64 ZIP contains a `Legal/` or `Documentation/` folder with: (a) `OpenSpatialDelay_Legal_Notices.pdf` (regenerated from the .docx or produce a PDF version), (b) a copy of the top-level `LICENSE` file. Regenerate and re-upload the v1.0.0 release asset after rebuilding. Also export a PDF version of the legal notices — .docx is not universally readable without Word/LibreOffice."
    files_affected: ["OpenSpatialDelay-v1.0.0-macOS-arm64.zip (release asset)", "packaging/release script (if exists, otherwise ad-hoc zip step)", "docs/generate_legal_notices.js (may need PDF export)"]
    severity: required_for_repo_02
deferred:
  - truth: "GitHub repo publicly visible"
    addressed_in: "Phase 3"
    evidence: "D-09: Repo stays private through Phase 1. D-10: REPO-02 redefined for Phase 1 as 'release prepared with macOS arm64 binary and metadata, ready to go public'. Phase 3 goal: 'The personal landing page is live' — making the repo public is bundled with Phase 3 delivery."
  - truth: "Windows binary attached to the v1.0.0 release"
    addressed_in: "Phase 3 (post)"
    evidence: "D-06: Windows binary deferred because CI tokens are exhausted. D-06b: Windows binary addition deferred to post-Phase 3."
---

# Phase 1: Repo & License Readiness — Verification Report (Updated)

**Phase Goal:** The codebase is clean GPL-3.0, the GitHub repo is public, the release is downloadable, and installation instructions are accurate
**Verified:** 2026-04-14T14:51:18Z
**Updated:** 2026-04-14T16:35:00Z (human review surfaced cross-repo decision + material delivery gap)
**Status:** gaps_found

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `git grep -i "commercial licen"` returns zero OSD results outside .planning/ and .claude/skills/ | VERIFIED | 2 matches remain in `docs/generate_legal_notices.js` and `docs/generate_manual.js` — both describe JUCE's own dual-license model, not OSD. Factually accurate. |
| 2 | GitHub v1.0.0 release is prepared with macOS arm64 binary and GPL-3.0 metadata (D-10 scope) | PARTIALLY VERIFIED | Release body GPL-3.0 link confirmed, but binary ZIP does not include LICENSE or Legal Notices — gap G-02. |
| 3 | README system requirements table exists | VERIFIED | Table present with tested DAWs per D-08. |
| 4 | Installation instructions include macOS Sequoia xattr command | VERIFIED | `### macOS security note (Gatekeeper / Sequoia)` with two `xattr -cr` commands. |

**Score:** 3/4 truths fully verified; Truth 2 has a material delivery-path gap (G-02).

### Gaps Found

See frontmatter `gaps:` for G-01 (strip legal section from manual) and G-02 (bundle legal notices into release ZIP). Both must be resolved to satisfy REPO-02 per the D-10 scope definition ("release prepared with macOS arm64 binary and metadata, ready to go public"). A release missing third-party notice compliance is not ready to go public.

### Deferred Items

Items explicitly addressed in later phases — unchanged from prior report.

| # | Item | Addressed In |
|---|------|-------------|
| 1 | GitHub repo publicly visible | Phase 3 (D-09) |
| 2 | Windows x64 binary attached to release | Phase 3+ (D-06b) |

### Human Verification — Resolved

The prior human_verification item ("Open manual PDF and confirm Project License reads GPL-3.0") is now superseded. The correct action per cross-repo decision is to strip the Project License section from the manual entirely (G-01) and deliver it via the separate `generate_legal_notices.js` document (G-02). The HUMAN-UAT item should be marked resolved when G-01 lands.

### Code Review Findings

Phase 1 code review produced 3 info findings (0 critical, 0 warning) — tracked in `01-REVIEW.md`. IN-02 ("generator output not regenerated") is absorbed into G-01 closure.

---

_Verified: 2026-04-14T14:51:18Z_
_Updated: 2026-04-14T16:35:00Z after user review surfaced cross-repo decision_
_Verifier: Claude (gsd-verifier + user collaboration)_
