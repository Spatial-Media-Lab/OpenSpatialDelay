# Specification — Release Documentation Review

## 1. Mission

Verify that every document shipping with OpenSpatialDelay v1.0.0 is factually consistent with the codebase, internally consistent across documents, and free of stale references from earlier development phases. The "done" picture: a report listing every discrepancy found (with fixes applied where possible), and a clear human-action-required list for issues needing subjective judgment (visual review, audio descriptions, screenshot freshness). The user can look at the report and know exactly what's left before pressing "ship."

## 2. Context

OpenSpatialDelay is a JUCE 8 (C++17) spatial delay plugin — macOS arm64 VST3+AU, Windows x64 VST3. v1.0.0 ships the week of 2026-04-10. The codebase has been through 30+ bug fix cycles since documentation was last comprehensively updated. The manual was generated from `docs/generate_manual.js` and marked COMPLETED on 2026-04-04, but 8 documentation issues were subsequently filed (#168-#176) and the code has continued to change (issues #178-#193 merged since then).

Two tracking databases exist in Notion:
- **OSD Documentation Review** (DB ID: `6c015fd8-337c-4d45-b104-af3826969d88`) — DOC-xxx issue tracking
- **OSD v1.0 Release Tests** (DB ID: `d6ad01a1-f5b5-419e-8749-0f9b1dbc4d4a`) — E-series test items

The codebase repository is at `~/conductor/workspaces/openspatialdelay/chiang-mai/` (Conductor workspace, branch `AndrewRahman/context-load`). GitHub remote: `Spatial-Media-Lab/OpenSpatialDelay`.

### Key numbers the documentation must agree on (ground truth is SOURCE CODE):

These are the "load-bearing facts" that appear across multiple documents and must be cross-checked:

| Fact | Ground truth location |
|------|----------------------|
| Number of output formats | `outputFormatRegistry` array in PluginProcessor.h |
| Number of algorithms | Algorithm class declarations in PluginProcessor.h |
| Number of HRTF profiles | HRTF profile array in PluginProcessor.cpp |
| Number of factory presets | `factoryPresets[]` array in PresetData.cpp |
| Number of trajectory shapes | TrajectoryEngine shape enum |
| Number of Catch2 tests | Run `./build/OpenSpatialDelayTests` and count |
| Parameter names/ranges | APVTS parameter creation in PluginProcessor.cpp |
| Developer name | `COMPANY_NAME` in CMakeLists.txt |
| Plugin version string | `PRODUCT_VERSION` in CMakeLists.txt |

## 3. Project-level success criteria

- [ ] Every factual claim in README.md verified against source code
- [ ] Every factual claim in RELEASE_NOTES_v1.0.0.md verified against source code
- [ ] Every factual claim in SPECIFICATION.md Phase 5 table verified against source code
- [ ] Every factual claim in RELEASE_PLAN_v1.0.0.md "Documentation Consistency Checklist" re-verified
- [ ] CLAUDE.md version registry, parameter list, and architecture notes verified against source
- [ ] All 11 wiki pages in docs/wiki/ cross-checked against source code
- [ ] All 8 open GitHub documentation issues (#168-#176) assessed and triaged
- [ ] NotionDB DOC-xxx items pulled and cross-referenced with GitHub issues
- [ ] Fixable discrepancies corrected in-place (with git commits)
- [ ] A human-action-required report produced listing all issues needing subjective review
- [ ] generate_manual.js factual claims spot-checked (algorithm names, format lists, parameter descriptions)
- [ ] generate_legal_notices.js license table verified against CLAUDE.md license table

## 4. Scope

### In scope
- All `.md` documentation files in repo root and `docs/`
- `docs/wiki/*.md` — 11 wiki pages
- `docs/generate_manual.js` — factual claims embedded in manual generator
- `docs/generate_legal_notices.js` — license table accuracy
- `README.md`
- `CLAUDE.md`
- `SPECIFICATION.md` (Phase 5 table and any claims about current state)
- `docs/RELEASE_NOTES_v1.0.0.md`
- `docs/RELEASE_PLAN_v1.0.0.md`
- `docs/RELEASE_TEST_CHECKLIST.md`
- `docs/VERSION_HISTORY.md` (most recent entries only)
- Open GitHub doc issues #168-#176
- NotionDB DOC-xxx items

### Out of scope
- Regenerating the manual PDF/DOCX (that requires `npm install` + `node generate_manual.js`)
- Updating screenshots (requires running the plugin UI — human task)
- Subjective judgment calls (HRTF character descriptions, creative tips accuracy)
- Code changes to the plugin itself
- Issue #59 (SpatialCore extraction — separate task)
- Non-documentation GitHub issues (#182, #183, etc.)

### Look-alikes (forbidden)
- **Fixing DSP bugs mentioned in docs** — verify the doc claim, don't fix the code
- **Rewriting documentation style** — check facts, don't rewrite prose
- **Updating SPECIFICATION.md roadmap dates** — the spec is a historical document; only verify Phase 5 "current state" claims
- **Modifying docs/generate_manual.js logic** — only verify embedded strings, don't refactor the generator

## 5. Anti-goals

- Do NOT refactor or restructure any documentation — only correct factual errors
- Do NOT add new documentation sections
- Do NOT modify frozen archive files (`Archive/`)
- Do NOT modify the plugin source code
- Do NOT regenerate the PDF manual (that's a separate human-triggered step)
- Do NOT close GitHub issues — report findings, let the user decide disposition
- Do NOT modify Notion DB entries — read only, report findings

## 6. Inputs and assets

| Name | Location | What it is | State | How to use it |
|---|---|---|---|---|
| Source code | `Source/*.h`, `Source/*.cpp` | Ground truth for all claims | Active development | Grep/Read for facts |
| README.md | repo root | Public-facing project description | Needs verification | Read + cross-check |
| SPECIFICATION.md | repo root | Full project spec (v1.1) | Partially stale | Verify Phase 5 table |
| CLAUDE.md | repo root | Claude development context | Active | Verify version registry, params |
| Release Notes | `docs/RELEASE_NOTES_v1.0.0.md` | User-facing release notes | Recently created | Verify all claims |
| Release Plan | `docs/RELEASE_PLAN_v1.0.0.md` | Release process checklist | Active | Verify doc consistency table |
| Test Checklist | `docs/RELEASE_TEST_CHECKLIST.md` | Manual test list | Active | Verify counts match code |
| Wiki pages | `docs/wiki/*.md` (11 files) | Detailed reference docs | Needs verification | Read + cross-check |
| Manual generator | `docs/generate_manual.js` | Node.js DOCX generator | Completed | Spot-check strings |
| Legal notices gen | `docs/generate_legal_notices.js` | License doc generator | Completed | Verify license table |
| Version history | `docs/VERSION_HISTORY.md` | Changelog | Active | Verify recent entries |
| Design philosophy | `docs/design-philosophy.md` | Architecture rationale | Historical | Spot-check |
| GitHub issues | #168-#176 | Open documentation issues | Open | Read via `gh issue view` |
| NotionDB (docs) | DB ID `6c015fd8...` | DOC-xxx issue tracker | Active | Read via Notion MCP |
| NotionDB (tests) | DB ID `d6ad01a1...` | E-series test tracker | Active | Cross-reference |
| CMakeLists.txt | repo root | Build config, version, company name | Active | Read for ground truth |
| Tests | `Tests/*.cpp` | Test files | Active | Count tests if needed |
| Trajectory PNGs | `docs/assets/trajectories/` (14 files) | Trajectory shape screenshots | Exist but unused in manual (#176) | Verify existence |

## 7. Constraints

- **Time budget:** 1 hour autonomous work (warn at 30 min, stop and report at 50 min)
- **Token budget:** ~500K tokens total across all agents (warn at 50%, stop at 80%)
- **Step budget:** 120 tool calls for orchestrator, 15-25 per subagent
- **Hard deadline:** Must complete before v1.0.0 ships (week of 2026-04-10)
- **External dependencies:** Notion MCP must be available for DOC-xxx retrieval. If unavailable, skip NotionDB tasks and note as gap.
- **No destructive actions:** All changes must be committed to a branch, never force-pushed to main

## 8. Quality bar

Production. The output report will be used to make ship/no-ship decisions. False positives (flagging something as wrong when it's correct) waste the user's time. False negatives (missing a real error) ship incorrect documentation to users. Err toward thoroughness over speed.

## 9. Definition of done (project-level)

The project is done when:
1. A file `outputs/doc_review_report.md` exists containing:
   - A summary of all discrepancies found (categorized as FIXED, HUMAN_REQUIRED, or INFO)
   - For FIXED items: the git commit hash of the fix
   - For HUMAN_REQUIRED items: specific instructions for what the human needs to verify
   - Cross-reference with all 8 open GitHub doc issues
   - Cross-reference with NotionDB DOC-xxx items (or a note that Notion was unavailable)
2. All FIXED items have been committed to the working branch
3. No known factual errors remain unfixed in any `.md` file (excluding subjective/visual issues)
