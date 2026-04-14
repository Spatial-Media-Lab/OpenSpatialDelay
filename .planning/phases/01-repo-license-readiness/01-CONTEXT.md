# Phase 1: Repo & License Readiness - Context

**Gathered:** 2026-04-14
**Status:** Ready for planning

<domain>
## Phase Boundary

Transition the codebase from dual-licensed (GPL-3.0 + Commercial) to GPL-3.0 only. Prepare the GitHub release with correct binaries. Document system requirements and installation. The repo stays private — making it public is deferred to Phase 3.

</domain>

<decisions>
## Implementation Decisions

### License Cleanup
- **D-01:** Full sweep across all 10 files that reference dual or commercial licensing. Every file gets updated to GPL-3.0 only — no stragglers.
- **D-02:** Files to update: LICENSE, README.md, CLAUDE.md, agent_docs/licensing.md, SPECIFICATION.md, docs/generate_manual.js, docs/generate_legal_notices.js, docs/RELEASE_NOTES_v1.0.0.md, docs/RELEASE_PLAN_v1.0.0.md, context-improvement-plan.md
- **D-03:** Source files (.cpp/.h) have no license headers — no changes needed there.
- **D-04:** Update the `.js` generator scripts (generate_manual.js, generate_legal_notices.js) but do NOT regenerate the manual PDF — user handles PDF regeneration manually.

### Release Assets
- **D-05:** v1.0.0 release must include macOS arm64 binary + Windows x64 binary. No separate macOS x86_64 build (Intel users use Rosetta 2).
- **D-06:** Windows binary is produced by triggering the existing `build-windows.yml` CI workflow on the v1.0.0 tag, then uploading the artifact to the GitHub Release.

### System Requirements
- **D-07:** System requirements go in a new section in README.md (not a separate file).
- **D-08:** List tested DAWs explicitly, plus a general "should work with any VST3/AU host" note. Don't promise compatibility with untested DAWs.

### Repo Visibility
- **D-09:** Repo stays private through Phase 1. The "make repo public" action is moved to Phase 3 scope.
- **D-10:** REPO-02 success criterion is redefined for Phase 1 as: "Release v1.0.0 is prepared with correct binaries and metadata, ready to go public." The "publicly visible" part is a Phase 3 deliverable.

### Claude's Discretion
- Order of file updates during license cleanup
- Exact wording of the GPL-3.0 license statement in each file (as long as it's accurate and consistent)
- System requirements table format and layout within the README section

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Licensing
- `agent_docs/licensing.md` — Current dual-license model and third-party license compatibility table. Update this to reflect GPL-3.0 only.
- `LICENSE` — Current dual-license file with full GPL-3.0 text. Replace header with GPL-3.0 only.

### Build & Release
- `.github/workflows/build-windows.yml` — Existing Windows CI workflow to trigger for producing the Windows binary
- `.github/workflows/build-macos.yml` — macOS CI workflow (already produced the arm64 build)
- `scripts/build_version.sh` — Versioned build script referenced in CLAUDE.md

### Documentation
- `docs/generate_manual.js` — Manual generator script with commercial license references to remove
- `docs/generate_legal_notices.js` — Legal notices generator with commercial license references to remove
- `README.md` — Primary documentation; license section and installation instructions need updates, system requirements section needs addition

### Success Criteria
- `SPECIFICATION.md` — Contains license references in 3 locations that need updating

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `build-windows.yml` CI workflow: Already configured for Windows x64 builds — can be triggered on the v1.0.0 tag
- `build-macos.yml` CI workflow: Already produced the arm64 binary attached to the release
- README.md installation section: Already has xattr commands for macOS Sequoia — needs system requirements table added above it

### Established Patterns
- Release naming: `OpenSpatialDelay-v1.0.0-{platform}-{arch}.zip` (per existing macOS arm64 asset)
- Source headers: No license headers in .cpp/.h files — this is the existing pattern, don't add any

### Integration Points
- GitHub Release v1.0.0: Already exists with macOS arm64 + source zip. Windows binary needs to be added as a new asset.
- CLAUDE.md: The "Why" section references dual-licensing — must be updated so future agent sessions see GPL-3.0 only

</code_context>

<specifics>
## Specific Ideas

- The success criterion for REPO-01 is `git grep -i "commercial licen"` returning zero results outside CHANGELOG — this is the verification command for the license sweep
- User wants the repo to be in a "flip the switch" state at end of Phase 1 — everything clean and ready so making public is a single `gh repo edit --visibility public` action in Phase 3

</specifics>

<deferred>
## Deferred Ideas

- **Make repo public** — Moved to Phase 3 (after website is live, so visitors have a full landing page to reach)
- **Manual PDF regeneration** — User handles this manually after generator scripts are updated
- **macOS x86_64 (Intel) binary** — Not included in v1.0.0; Rosetta 2 covers Intel users. Can be added to a future release if requested.

</deferred>

---

*Phase: 01-repo-license-readiness*
*Context gathered: 2026-04-14*
