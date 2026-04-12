# Context Improvement — Session Handoff

**Created:** 2026-04-11 (updated 2026-04-12)
**Purpose:** Paste-ready briefing for the next Claude Code session to resume Recommendation #5 (skill audit) at the exact point it was paused.
**Related:** `context-improvement-plan.md`, `context-audit-report.md`

---

Resuming the OpenSpatialDelay context-improvement audit, specifically Recommendation #5 (skill audit). Full state is in `context-improvement-plan.md` under "### 5. Audit and consolidate user-level skills" and in the `feedback_*` and `reference_*` memory files.

## Where we are

- **Phase 1 complete** (project-level duplicate removal, commit `bd39752`)
- **Phase 2 — Groups A + B PARTIALLY EXECUTED (2026-04-12).**
  - Decision rationale: user has many non-music projects; listing overhead per non-music session isn't worth it. Accept N copies across N JUCE projects.
  - **9 skills moved to OSD project level:** `adm-osc-integration`, `dsp-cookbook`, `spatial-audio-dsp`, `time-based-effects`, `cross-platform-builds`, `daw-compatibility-guide`, `juce-best-practices`, `plugin-architecture-patterns`, `synth-ui-components`
  - **6 skills NOT moved to OSD** (remain user-level, copied to SpatialCore only): `fm-synthesis-deep-dive`, `reverb-algorithms`, `synthesis-techniques`, `wavetable-engineering`, `spatial-synth-architecture`, `spatialcore-architecture`
  - **All 15 skills copied to SpatialCore** (`~/conductor/repos/SpatialCore/.claude/skills/`) — not yet committed/pushed there.
  - **User-level copies still exist** — deletion pending user approval per working rules.
- **Group J** — ✓ RESOLVED. `docx`/`pdf` replaced by `document-skills` plugin; `canvas-design`/`doc-coauthoring` moved to user-level (removed from OSD project-level 2026-04-12); `skill-creator` added to user-level.
- **Group C review — NOT STARTED.** ~24 utility/workflow/Notion/superpowers skills still need keep/move/disable decisions.
- **Group K (GSD)** — deferred post-v1.0.
- **Stale 12.0.0 symlink — CLEANED UP** (removed at start of 2026-04-12 session).

## Working rules — READ BEFORE TOUCHING ANYTHING

1. **DO NOT delete, disable, or move ANY skill, file, or config without explicit per-item human approval.** Report findings and propose actions; wait for a go/no-go per item. "Continue" or "proceed" is NOT blanket authorization. See `feedback_no_destructive_ops_without_approval.md` — **three prior incidents this week**.
2. **Check cascade semantics AND in-memory state before running any `claude plugin *` command, or before deleting any plugin install directory.** See `reference_claude_code_plugin_system.md`.
3. **No marketplace operations without first reading** `reference_claude_code_plugin_system.md`.
4. **Never delete a plugin install directory that any currently-running Claude Code session might still reference in memory.**

## Action sequence for next session

### Step 1: Move Groups A + B to project level — ✓ DONE (2026-04-12)

- 9 skills copied to OSD project level, 15 skills copied to SpatialCore project level
- `canvas-design` and `doc-coauthoring` moved from OSD project level to user level
- User-level copies of all 15 A+B skills still exist — deletion pending user approval
- SpatialCore changes not yet committed/pushed

### Step 2: Resume Group C review

Skills still needing keep/move/disable decisions (excluding GSD and plugin skills):

**C. General Engineering (6):**
`design-an-interface`, `improve-codebase-architecture`, `multi-plugin-conductor`, `pair-programming`, `setup-pre-commit`, `triage-issue`

**D. Process / Thinking (4):**
`deep-research`, `grill-me`, `rubber-duck`, `ubiquitous-language`

**E. Git Safety (1):**
`git-guardrails-claude-code`

**F. PRD / Planning (4):**
`prd-to-issues`, `prd-to-plan`, `request-refactor-plan`, `write-a-prd`

**G. UI / Design (3):**
`brand-forge`, `macos-design`, `oiloil-ui-ux-guide`

**H. Content / Notes / Research (4):**
`edit-article`, `last30days`, `notion-sml`, `obsidian-vault`

**I. Meta (2):**
`skill-builder`, `pages`

**J. OSD Project-Level — ✓ RESOLVED:**
`docx`, `pdf` replaced by `document-skills` plugin; `canvas-design`, `doc-coauthoring` kept as project-level; `skill-creator` added to user-level

Review format: user will direct the approach (may not want the full 6-point per-skill format — ask first).

## Outstanding items

- `brand-forge` — restored from user backup 2026-04-12. Full 6-phase pipeline intact. Categorized under Group C (general-purpose, keep at user level).
- Group K (GSD) strategy — deferred post-v1.0.
- `thedotmack/claude-mem` GH issue draft — internal-only in the plan file, not to be posted without user direction.

## Start now by

0. **Report the full skill inventory status.** Before doing anything else, print a table of ALL skills that still need action — both decided-but-not-executed (Groups A+B) and undecided (Group C–I). Format:

   | Group | Skill | Decision | Executed? |
   |-------|-------|----------|-----------|
   | A (DSP) | `adm-osc-integration` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | A (DSP) | `dsp-cookbook` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | A (DSP) | `spatial-audio-dsp` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | A (DSP) | `time-based-effects` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | A (DSP) | `fm-synthesis-deep-dive` | SpatialCore only (not OSD) | Yes (SC only) |
   | A (DSP) | `reverb-algorithms` | SpatialCore only (not OSD) | Yes (SC only) |
   | A (DSP) | `synthesis-techniques` | SpatialCore only (not OSD) | Yes (SC only) |
   | A (DSP) | `wavetable-engineering` | SpatialCore only (not OSD) | Yes (SC only) |
   | B (Infra) | `cross-platform-builds` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | B (Infra) | `daw-compatibility-guide` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | B (Infra) | `juce-best-practices` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | B (Infra) | `plugin-architecture-patterns` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | B (Infra) | `synth-ui-components` | Move to OSD + SpatialCore project level | Yes (OSD + SC) |
   | B (Infra) | `spatial-synth-architecture` | SpatialCore only (not OSD) | Yes (SC only) |
   | B (Infra) | `spatialcore-architecture` | SpatialCore only (not OSD) | Yes (SC only) |
   | J | `canvas-design` | Move to user level | Yes |
   | J | `doc-coauthoring` | Move to user level | Yes |
   | C (Engineering) | `design-an-interface` | Undecided | — |
   | C (Engineering) | `improve-codebase-architecture` | Undecided | — |
   | C (Engineering) | `multi-plugin-conductor` | Undecided | — |
   | C (Engineering) | `pair-programming` | Undecided | — |
   | C (Engineering) | `setup-pre-commit` | Undecided | — |
   | C (Engineering) | `triage-issue` | Undecided | — |
   | D (Process) | `deep-research` | Undecided | — |
   | D (Process) | `grill-me` | Undecided | — |
   | D (Process) | `rubber-duck` | Undecided | — |
   | D (Process) | `ubiquitous-language` | Undecided | — |
   | E (Git Safety) | `git-guardrails-claude-code` | Undecided | — |
   | F (PRD/Planning) | `prd-to-issues` | Undecided | — |
   | F (PRD/Planning) | `prd-to-plan` | Undecided | — |
   | F (PRD/Planning) | `request-refactor-plan` | Undecided | — |
   | F (PRD/Planning) | `write-a-prd` | Undecided | — |
   | G (UI/Design) | `brand-forge` | Undecided | — |
   | G (UI/Design) | `macos-design` | Undecided | — |
   | G (UI/Design) | `oiloil-ui-ux-guide` | Undecided | — |
   | H (Content) | `edit-article` | Undecided | — |
   | H (Content) | `last30days` | Undecided | — |
   | H (Content) | `notion-sml` | Undecided | — |
   | H (Content) | `obsidian-vault` | Undecided | — |
   | I (Meta) | `skill-builder` | Undecided | — |
   | I (Meta) | `pages` | Undecided | — |

   Update the "Executed?" column as skills are completed. Remove rows once fully done (moved + user-level deleted with approval).

1. Reading `context-improvement-plan.md` Rec #5 section to confirm current state.
2. Reading `feedback_no_destructive_ops_without_approval.md` and `reference_claude_code_plugin_system.md`.
3. Executing Step 1 (move 15 JUCE skills to project level) with per-item confirmation.
4. Then proceeding to Step 2 (Group C review) — ask the user how they want to go through them before starting.

Do not start any marketplace operations, plugin installs, or file deletions during this session unless explicitly requested with a concrete target.
