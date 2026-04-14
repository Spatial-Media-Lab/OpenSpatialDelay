# Context Improvement Plan

**Date:** 2026-04-10
**Based on:** `context-audit-report.md` (same date)
**Prepared by:** Claude (context engineering strategist role)

---

## Executive Summary

| Metric | Current | Target | Change |
|--------|---------|--------|--------|
| Session-start tokens | ~21,135 | ~11,800 | -9,335 (~44% reduction) |
| CLAUDE.md tokens | ~2,100 | ~1,200 | -900 |
| MEMORY.md tokens | ~3,400 | ~600 | -2,800 |
| Skill listing tokens | ~5,100 | ~2,500 | -2,600 |
| Deferred tool names | ~1,000 | ~700 | -300 |

**Expected performance impact:** On a 1M-token Opus window, current overhead is only ~2.1% — this is not an emergency. However, context size affects more than capacity: Anthropic's own research confirms that model precision for retrieval and reasoning degrades as context fills, even when well within the window limit [Source 1]. Shorter system prompts also improve cache hit rates (the cache boundary sits between static and session-specific content) [Source 4]. The primary benefit of these changes is **signal-to-noise ratio** — every unnecessary token in the system prompt is a distraction from the instructions that actually matter.

---

## Prioritized Recommendations

Ordered by savings-to-risk ratio (highest-value, lowest-risk first).

### 1. Restructure MEMORY.md from knowledge base to lean index

**The problem:** MEMORY.md is 11,850 bytes (~3,400 tokens), loaded every session. The auto-memory system instructions explicitly state: *"MEMORY.md is an index, not a memory — each entry should be one line, under ~150 characters."* The current file violates this design — it contains 13 full content sections (Project Identity, Key Files, Build System, Architecture, Parameters, etc.) totaling ~3,000 tokens that belong in individual memory files, not the index.

**Estimated savings:** ~2,800 tokens (reduce MEMORY.md from ~3,400 to ~600 tokens)

**Risk:** Low. The content is preserved in individual memory files that load on demand when relevant. Claude still sees the one-line index entry and can pull details when needed.

**Best practice alignment:** The auto-memory system's own specification: index entries should be one line, under 150 characters [System prompt, auto-memory section]. Also: "For each line, ask: Would removing this cause Claude to make mistakes? If not, cut it" [Source 2].

**Exact edit:** Replace the current MEMORY.md with a lean index. Move each content section into its own memory file:

```
# New memory files to create:
project_identity.md          — from §"Project Identity" (plugin name, author, purpose, target, GitHub URL)
project_key_files.md         — from §"Key Files" (file paths and sizes)
project_build_system.md      — from §"Build System" (cmake commands, deps, preset location)
project_frozen_builds.md     — from §"Frozen Builds" (v0.1-v0.9 archive details)
project_architecture.md      — from §"Architecture" + §"Key Architecture Details"
project_structure.md         — from §"Project Structure"
project_status.md            — from §"Current Status"
project_parameters.md        — from §"Parameters"
project_version_history.md   — from §"Version History Summary" + §"Version Freeze SOP"
user_preferences.md          — from §"Critical User Preferences"
reference_spatialization.md  — from §"SpatializationAlgorithm Interface" + §"HRTF Profiles"

# Then rewrite MEMORY.md as a pure index (~600 tokens):
- [Project Identity](project_identity.md) — OSD v1.0, spatial delay plugin, macOS arm64 + Windows CI
- [Key Files](project_key_files.md) — 6 source files, PluginProcessor.cpp is ~175KB
- [Build System](project_build_system.md) — JUCE 8.0.3, CMake, libmysofa, build_version.sh workflow
- [Frozen Builds](project_frozen_builds.md) — v0.1-v0.9 on GitHub Releases, never modify
- [Architecture](project_architecture.md) — 5-stage signal flow, HRTF convolution, lock-free processBlock
- [Project Structure](project_structure.md) — repo layout under ~/conductor/repos/openspatialdelay/
- [Current Status](project_status.md) — v1.0 active dev, 162 tests, 22 output formats
- [Parameters](project_parameters.md) — global + per-object (x12) + non-APVTS list
- [Version History](project_version_history.md) — v0.1-v0.9 frozen, v1.0 active, freeze SOP
- [User Preferences](user_preferences.md) — VST3+AU only, no sudo, frozen versions final
- [Spatialization Reference](reference_spatialization.md) — 7 algorithms, 6 HRTF profiles
- [Versioned Builds](feedback_versioned_builds.md) — CRITICAL: use build_version.sh
- [No AU Cache Refresh](feedback_no_au_cache_refresh.md) — NEVER run killall AudioComponentRegistrar
- [Version Discipline](feedback_version_discipline.md) — Stay on same version while iterating
- [Version Comparison](feedback_version_comparison.md) — v1.0 == v1.0.0
- [No Dual-Head Pitch](feedback_no_dual_head_pitch.md) — Keep WSOLA-Lite, never dual-head crossfade
- [HRTF Profile Names](feedback_hrtf_profile_names.md) — Use UI names not dataset names
- [Push Documentation](feedback_push_documentation.md) — Always document pushes
- [Version Reset](project_version_reset.md) — Registry collapsed to v1.0.0 on 2026-04-01
- [Ship Date](project_ship_date.md) — v1.0.0 target end of week 2026-04-10
- [Notion Workspace](reference_notion_workspace.md) — Pages under "Claude's space"
- [Release Tests DB](reference_notion_release_tests_db.md) — OSD v1.0 Release Tests NotionDB
- [Doc Review DB](reference_notion_doc_review_db.md) — OSD Documentation Review NotionDB
```

### 2. Remove 16 duplicated project-level skills

**The problem:** 16 skills exist in both `~/.claude/skills/` (user-level) and `.claude/skills/` (project-level) with identical names. Both instances appear in the skill listing, inflating it. The project-level copies are redundant since user-level skills are already available in every project.

**Estimated savings:** ~800 tokens from skill listing (16 skills x ~50 tokens each for name+description)

**Risk:** Low. The user-level copies remain. If any project-level copies have been customized for this project, a diff should be run first to verify.

**Best practice alignment:** Avoid duplicate skill registrations that bloat the skill index [Source 3]. Project-level skills should only exist for project-specific customizations.

**Exact steps:**
```bash
# 1. First, verify they're identical (run diffs before deleting):
for skill in code-quality-checker cross-platform-builds daw-compatibility-guide dead-code-auditor dsp-cookbook juce-best-practices macos-design oiloil-ui-ux-guide plugin-architecture-patterns reverb-algorithms runtime-performance-auditor sound-engineer spatial-audio-dsp synthesis-techniques time-based-effects verification-before-completion; do
  diff -q ~/.claude/skills/$skill/SKILL.md .claude/skills/$skill/SKILL.md 2>/dev/null
done

# 2. If identical, remove the project-level duplicates:
# (only the 16 confirmed duplicates — keep the 6 project-only skills)
```

### 3. Slim CLAUDE.md: externalize version registry

**The problem:** The version registry table in CLAUDE.md is ~550 tokens (13 entries + header). It grows with every fix. It serves one purpose: preventing version number reuse. Claude can check a reference file for this; it does not need to be in the always-loaded system prompt.

**Estimated savings:** ~550 tokens

**Risk:** Medium-low. The critical instruction ("Next available: v1.0.21 / O121") must remain in CLAUDE.md as a single line. The full registry moves to a file that Claude reads when building.

**Best practice alignment:** "For each line, ask: Would removing this cause Claude to make mistakes?" — Claude only needs the *next available* number in the system prompt, not the full history [Source 2].

**Exact edit:** In `CLAUDE.md`, replace the full registry table with:

```markdown
**Version number registry:** See `docs/VERSION_REGISTRY.md` for full history.
**Next available: v1.0.21 / O121** — always check the registry before building.
```

Create `docs/VERSION_REGISTRY.md` containing the full table (moved from CLAUDE.md).

### 4. Slim CLAUDE.md: externalize licensing tables

**The problem:** The "Licensing & Monetization" section is ~450 tokens. It's reference material consulted during licensing decisions, not something needed every session.

**Estimated savings:** ~450 tokens

**Risk:** Low. Licensing decisions are infrequent. Claude can read the reference file when needed.

**Best practice alignment:** Same as #3 — keep CLAUDE.md focused on build commands, style rules, and critical workflow rules. Reference material belongs in docs [Source 2].

**Exact edit:** In `CLAUDE.md`, replace the licensing line with:

```markdown
Licensed under GPL-3.0. See LICENSE for full text.
```

Move the full licensing reference content to `agent_docs/licensing.md`.

### 5. Audit and consolidate user-level skills

**The problem:** 147 skills are listed at session start (~5,100 tokens). Of these, ~60 are GSD workflow skills and ~10 are audio/JUCE domain skills. For a project that's 95% C++ audio plugin work, skills like `migrate-to-shoehorn`, `scaffold-exercises`, `brand-forge`, `make-scenario-builder`, and `canvas-design` are noise.

**Estimated savings:** ~1,200 tokens if ~25 irrelevant skills are disabled or moved to a separate profile.

**Risk:** Medium. Disabling user-level skills affects all projects. This requires judgment about which skills the user may want in other projects. An alternative is project-level skill exclusion if Claude Code supports it.

**Best practice alignment:** "Claude Code's skill listing costs ~50 tokens per skill. At 100+ skills, this becomes a meaningful fraction of system prompt budget" — keep active skill count to what the current project actually uses [Source 3].

---

#### Status (as of 2026-04-11)

**Phase 1 — project-level duplicate removal** ✓ Complete
- Commit `bd39752` removed 17 duplicate project-level skills
- Project skills reduced from ~22 → 4: `canvas-design`, `doc-coauthoring`, `docx`, `pdf`
- Also removed `sound-engineer` (out of scope — music production, not plugin DSP). User-level copy also removed 2026-04-12.
- Also consolidated `spatial-audio-dsp` (merged project-level enhancements into user-level)
- Actual savings: ~2,100 tokens (262% of the ~800 estimated)

**Phase 2 — user-level skill audit** ✓ Complete (2026-04-12)

*Working rule (set 2026-04-11 after a destructive incident):* **Claude does NOT delete, disable, or move any skill without explicit human approval.** Report and suggest only. The initial auto-delete of 7 skills caused recovery work for `last30days`, `edit-article`, `obsidian-vault` (restored) and `brand-forge` (restored from backup 2026-04-12).

*Group A — DSP algorithm skills — MOVE to project level* (decision 2026-04-12)
- `dsp-cookbook`, `reverb-algorithms`, `spatial-audio-dsp`, `synthesis-techniques`, `time-based-effects`
- **Added 2026-04-12 (backup restore):** `adm-osc-integration`, `fm-synthesis-deep-dive`, `wavetable-engineering`
- OSD-specific references pruned from SKILL.md files during earlier review
- **Origin audit (2026-04-12):** Original 5 are user-authored. No upstream exists on GitHub. Zero drift risk. 3 new skills restored from backup — origin audit pending.
- **Decision:** Move from `~/.claude/skills/` to `.claude/skills/` in each JUCE plugin project. Rationale: the user has many non-music projects; ~450+ tokens of listing overhead per non-music session is not worth it given zero maintenance burden (nothing to sync). Accept N copies across N JUCE projects as the cost.
- **Status:** ✓ Executed 2026-04-12. Copied to OSD + SpatialCore project level. User-level copies retained.

*Group B — JUCE/plugin infra skills — MOVE to project level* (decision 2026-04-12)
- `cross-platform-builds`, `daw-compatibility-guide`, `juce-best-practices`, `plugin-architecture-patterns`
- **Added 2026-04-12 (backup restore):** `spatialcore-architecture`, `spatial-synth-architecture`, `synth-ui-components`
- **Origin audit (2026-04-12):**
  - `cross-platform-builds` — from `yebot/rad-cc-plugins`, condensed rewrite by user
  - `daw-compatibility-guide` — from `yebot/rad-cc-plugins`, heavily rewritten by user (66 lines vs 992 upstream)
  - `juce-best-practices` — from `yebot/rad-cc-plugins`, verbatim except frontmatter tweak
  - `plugin-architecture-patterns` — from `yebot/rad-cc-plugins`, condensed rewrite by user
  - `spatialcore-architecture` — origin audit pending (restored from backup)
  - `spatial-synth-architecture` — origin audit pending (restored from backup)
  - `synth-ui-components` — origin audit pending (restored from backup)
- **Decision:** Move to project level alongside Group A. Same rationale: user's versions are intentionally condensed and won't track upstream. Drift risk is near-zero. Accept N copies across JUCE projects.
- **Status:** ✓ Executed 2026-04-12. Copied to OSD + SpatialCore project level. User-level copies retained.

*Group K — GSD workflow skills — KEEP at user level, re-evaluate post-v1.0*
- 66 `gsd-*` skills are candidates for plugin-architecture conversion later
- Decision deferred until after v1.0.0 ships (2026-04-10)

*Group C–I — utility / workflow / Notion / superpowers skills — ✓ REVIEWED (2026-04-12)*

**C. General Engineering (6):**
1. `design-an-interface`
2. `improve-codebase-architecture`
3. `multi-plugin-conductor` — restored from backup 2026-04-12
4. `pair-programming`
5. `setup-pre-commit`
6. `triage-issue`

**D. Process / Thinking (4):**
1. `deep-research`
2. `grill-me`
3. `rubber-duck`
4. `ubiquitous-language`

**E. Git Safety (1):**
1. `git-guardrails-claude-code`

**F. PRD / Planning (4):**
1. `prd-to-issues`
2. `prd-to-plan`
3. `request-refactor-plan`
4. `write-a-prd`

**G. UI / Design (3):**
1. `brand-forge` — restored from backup 2026-04-12
2. `macos-design`
3. `oiloil-ui-ux-guide`

**H. Content / Notes / Research (4):**
1. `edit-article`
2. `last30days`
3. `notion-sml`
4. `obsidian-vault`

**I. Meta (2):**
1. `skill-builder`
2. `pages`

**Decisions (2026-04-12):**
- C (Engineering, 6), D (Process, 4), E (Git Safety, 1), F (PRD/Planning, 4), H (Content, 4), I (Meta, 2) — **keep at user level** (21 skills, no changes needed)
- G (UI/Design, 3: `brand-forge`, `macos-design`, `oiloil-ui-ux-guide`) — **copied to project level** in `sml-website-redesign` + `website-rework`, pushed to both remotes. User-level copies retained.

*Group J — OSD project-level skills (`.claude/skills/`) — ✓ RESOLVED 2026-04-12*
- **Installed `document-skills@anthropic-agent-skills` plugin** — provides `docx`, `pdf`, `xlsx`, `pptx` at user level via plugin
- **Removed project-level copies** of `docx` and `pdf` (now redundant with plugin)
- **Kept project-level copies** of `canvas-design` and `doc-coauthoring` (not installing `example-skills` plugin — its 12 skills would add too much listing overhead for just 2 needed skills)
- **Added `skill-creator` to user-level** (`~/.claude/skills/skill-creator/`) — copied from `anthropic-agent-skills` marketplace. Complements existing `skill-builder` (spec reference vs iterative dev workflow with eval tooling).
- **Comparison:** `skill-builder` = how to write SKILL.md correctly (spec/templates); `skill-creator` = iterative skill development with quantitative eval benchmarking. Both kept at user level.

**Marketplace additions (2026-04-11)** — for future plugin hunting, not yet installed:
- `wshobson/agents` → `claude-code-workflows` (77 plugins, 33.4k ⭐) — registered, uninstalled
- `anthropics/skills` → `anthropic-agent-skills` (17 skills / 3 plugins, 115k ⭐) — registered, uninstalled; replacement path for Group J
- `davila7/claude-code-templates` → `claude-code-templates` (10 plugins, 24.4k ⭐) — **added then removed** because the 150 MB monorepo clone was not worth the ~7 MB of actual plugin content. Can be re-added later with `--sparse .claude-plugin plugins` if we decide we want specific plugins.

**Open items**
1. ~~`brand-forge` recovery still blocked by macOS TCC~~ — **RESOLVED 2026-04-12:** restored from user backup. Full 6-phase pipeline intact (11 files). Categorized under Group C (general-purpose, keep at user level).
2. GSD plugin strategy — defer until post-v1.0
3. Group C audit — next up
4. Group J replacement via `anthropic-agent-skills` — queued for when Group C review reaches project-level skills
5. **Disk hygiene — thedotmack/claude-mem** (outside context-plan scope, filed here for reference):

    **Problem found (2026-04-11):** `thedotmack/claude-mem` was consuming ~1.23 GB on disk (628 MB marketplace clone + 604 MB install cache). Root cause: the upstream marketplace repo commits `plugin/node_modules/` to git (525 MB, mostly tree-sitter language parsers), and the install cache duplicates it.

    **What we tried:**

    - **Attempt 1 — `claude plugin marketplace remove thedotmack` to clean up.** This was a mistake: `marketplace remove` **cascade-uninstalls all plugins from that marketplace**, not just the clone. `installed_plugins.json` lost the claude-mem entry and `settings.json` lost all claude-mem hooks — plugin became non-functional while 604 MB of cache remained orphaned on disk.
    - **Attempt 2 — R2 sparse re-add:** `claude plugin marketplace add thedotmack/claude-mem --sparse .claude-plugin plugin/hooks plugin/skills plugin/modes plugin/ui plugin/scripts plugin/CLAUDE.md plugin/package.json plugin/bun.lock plugin/.claude-plugin`. This briefly produced an 89 MB marketplace clone. Then followed by `claude plugin enable claude-mem@thedotmack`, which added the entry to `settings.json` `enabledPlugins` dict without running a formal `install`.
    - **What defeated the sparse approach:** claude-mem ships a `SessionStart` hook that runs `scripts/smart-install.js`, which detects missing `node_modules/` and runs `bun install` automatically. Within seconds of enabling the plugin, the marketplace clone was rebuilt to 626 MB. **Sparse checkout cannot beat a plugin that self-bootstraps its own dependencies at session start.**
    - **Attempt 3 — `rm -rf cache/thedotmack/claude-mem/12.0.0`** to remove the orphaned 604 MB install cache. This succeeded. The same session-start hooks also triggered an auto-update from 12.0.0 → 12.1.0 (the current upstream), installing a new thin 65 MB cache in place (no `node_modules/` in the new cache because the plugin resolves deps from the marketplace clone via bun).

    **Final state:**
    - Marketplace clone: 626 MB (was 628 MB — effectively identical, `node_modules` regenerates at every session start)
    - Install cache 12.1.0: 65 MB (down from 604 MB of 12.0.0)
    - Net disk reclaim: **~541 MB** for thedotmack alone (~689 MB total session reclaim including davila7)
    - Worker service on :37777: healthy throughout
    - claude-mem v12.1.0 fully functional

    **Third mistake (same session, shortly after) — stale in-memory hook path cache:** Deleting `cache/thedotmack/claude-mem/12.0.0/` felt safe because (a) the worker service was running from the marketplace clone, (b) 12.1.0 had auto-installed as a newer thin cache, (c) `claude plugin validate` passed. **What I missed:** Claude Code caches plugin install paths in memory at session start, and mid-session plugin auto-updates (like claude-mem's smart-install going 12.0.0 → 12.1.0) do NOT refresh this in-memory cache. The current session still held hook references to the now-deleted 12.0.0 path. Every subsequent turn triggered a Stop hook validation error: `"Plugin directory does not exist: .../claude-mem/12.0.0 (claude-mem@thedotmack — run /plugin to reinstall)"`. The error is cosmetic (doesn't block turns, worker service still healthy) but annoying as hell for the user.

    **Fix — symlink workaround (F1):**
    ```bash
    ln -s 12.1.0 /Users/andrewrahman/.claude/plugins/cache/thedotmack/claude-mem/12.0.0
    ```
    Creates a symlink at the old path pointing at the new version. Claude Code's validator now finds the "directory" via the symlink, hooks resolve to 12.1.0's content, error stops. Reversible with `rm` on the symlink only. Cleaner fix (F2) is a session restart — the new session will re-resolve plugin paths from scratch and the symlink can be removed.

    **Session state right now:**
    - Symlink `cache/thedotmack/claude-mem/12.0.0 → 12.1.0` is in place
    - Next fresh session will not need it; can be removed after session restart

    **Takeaways filed to memory:**
    - `claude plugin marketplace remove` cascade-uninstalls all plugins from that marketplace (`feedback_no_destructive_ops_without_approval.md`)
    - `claude plugin marketplace add --sparse` is defeated by plugins that self-bootstrap dependencies via SessionStart hooks (`reference_claude_code_plugin_system.md`)

    **GitHub issue draft (per 2026-04-11 decision: NOT to be posted — kept here as internal reference only):**

    > **Title:** Marketplace repo ships ~525 MB of committed node_modules — redundant with plugin's own `smart-install.js` bootstrap
    >
    > **Summary:** `plugin/node_modules/` is committed to git (~525 MB, tree-sitter parsers dominate). This is redundant because the plugin already has `plugin/scripts/smart-install.js` which runs in the SessionStart hook and performs `bun install` when `node_modules/` is missing. Any user cloning via `--sparse` (excluding `node_modules/`) gets a working plugin within seconds as `smart-install.js` rebuilds the deps from `bun.lock`. The committed tree in git is dead weight that forces a ~628 MB download on `claude plugin marketplace add` for every new user.
    >
    > **Repro:** `claude plugin marketplace add thedotmack/claude-mem` → 628 MB download. `du -sh ~/.claude/plugins/marketplaces/thedotmack/plugin/node_modules/` → 525 MB committed.
    >
    > **Proposed fix:**
    > 1. Add `node_modules/` to `.gitignore`
    > 2. Rely on existing `smart-install.js` bootstrap (already does the right thing)
    > 3. Optional: add an explicit `Setup` hook that runs `bun install` on first plugin activation (redundant with SessionStart, but faster first-run experience)
    >
    > **Biggest offenders inside node_modules:** `tree-sitter-swift` 21 MB, `tree-sitter-cpp` 9.1 MB, `tree-sitter-typescript` 6 MB, plus tree-sitter parsers for go/haskell/java/js/php/python/ruby/rust/scala/scss.
    >
    > **Environment:** claude-mem v12.0.0 → v12.1.0 auto-update observed, Claude Code CLI, macOS arm64.

    **Action items for this project:** none. Accept ~700 MB as the irreducible cost of claude-mem until upstream fix. Revisit if disk pressure becomes a concern.

---

**Original exact-steps list (superseded by Group C review protocol above):**
```
# Skills that appeared irrelevant to this C++ audio plugin project:
# (Some have been decided, some were auto-deleted and recovered, some still pending)
migrate-to-shoehorn     # TypeScript-specific          — DELETED in Phase 2 incident
scaffold-exercises      # Course/exercise creation     — DELETED in Phase 2 incident
brand-forge             # Brand identity design        — DELETED, recovery blocked
make-scenario-builder   # Make.com automation          — DELETED in Phase 2 incident
canvas-design           # Visual art creation          — PENDING review
last30days              # Social media research        — RECOVERED, pending review
truth:start             # Truth command                — PENDING review
stream-chain:pipeline   # Stream chain                 — DELETED in Phase 2 incident
stream-chain:run        # Stream chain                 — DELETED in Phase 2 incident
examples:tdd-examples   # TDD examples                 — DELETED in Phase 2 incident
.github:CODE_OF_CONDUCT # Code of conduct              — DELETED in Phase 2 incident
.github:CONTRIBUTING    # Contributing guide           — PENDING review
README                  # README viewer                — PENDING review
```

### 6. Evaluate disabling or consolidating dual Notion integrations — RESOLVED 2026-04-12

**Outcome:** Cannot consolidate — the two integrations serve different Notion workspaces:
- `mcp__notion-sml__*` (22 tools) → **Spatial Media Lab e.V.** workspace (Funding Opportunities, Shitty Music Catalog)
- `mcp__claude_ai_Notion__*` (14 tools) → **Andrew Rahman HQ** workspace (OSD Release Tests, OSD Documentation Review)

Neither can replace the other. The original premise ("overlapping integrations") was incorrect — they overlap in functionality but access mutually exclusive workspaces. The `notion-sml` API version was already at `2025-09-03`, so all 22 tools work natively (no curl workarounds needed).

**Additional actions taken:**
- Added `notion-sml` MCP server to `claude_desktop_config.json` (was missing — Claude Desktop had no MCP servers configured)
- Removed `~/.claude/skills/notion-sml/` skill file — the MCP server replaces it; routing info lives in memory files

**Actual savings:** ~0 tokens from consolidation (both integrations required). Minor savings from removing the skill file from the skills listing.

### 7. Remove CLAUDE.md ↔ MEMORY.md content overlap — DEFERRED

**Status:** Deferred 2026-04-12. CLAUDE.md was already slimmed during skill audit work — no architecture content remains in it. Overlap now runs the other direction: memory files (`project_structure.md`, `project_key_files.md`, `project_build_system.md`) duplicate content that CLAUDE.md covers as instructions. Proposed fix is to trim from the memory side (~600 tokens). Come back when convenient.

**The problem (after recommendation #1 is applied):** Even after slimming MEMORY.md to an index, the individual memory files will still overlap with CLAUDE.md. Specifically:
- Build commands appear in both CLAUDE.md and what would become `project_build_system.md`
- Architecture details appear in both CLAUDE.md and `project_architecture.md`
- Version history appears in both CLAUDE.md and `project_version_history.md`

**Estimated savings:** ~400 tokens (from CLAUDE.md, by removing architecture details that duplicate what's in memory)

**Risk:** Medium. CLAUDE.md should remain the authoritative source for *instructions* (build commands, mandatory workflows). Memory files should hold *context* (what the project is, how it's structured). The line between "instruction" and "context" requires judgment.

**Proposed split:**
- **Keep in CLAUDE.md:** Build commands, versioned build mandate, test commands, "next available version" line
- **Move to memory only:** Architecture notes (HRTF signal flow, PartitionedConvolver, Phase Vocoder details, Dry Path Compensation, ITD delay line) — these are reference material Claude consults when working on DSP, not instructions it needs every session

**Best practice alignment:** "Include what Claude cannot infer from the code; exclude what it can discover by reading files" [Source 2]. Architecture details are discoverable from the source code and code comments.

### 8. Evaluate disabling Gmail and Google Calendar integrations — DEFERRED

**Status:** Reviewed 2026-04-12. Not in use, but token impact is negligible (~50 tokens). Decision: keep enabled, revisit if tool listing noise becomes a concern.

**The problem:** Two built-in integrations (Gmail, Google Calendar) expose 2 deferred tools total. They require authentication and have never been used in this project context.

**Estimated savings:** ~50 tokens (minimal)

**Risk:** Very low. Can be re-enabled if needed.

**Best practice alignment:** "Disable unused MCP integrations to reduce tool listing noise" [Source 5].

**Exact steps:** Disable via Claude Code settings or Claude.ai integration settings.

---

## Structural Recommendations

### A. Adopt a CLAUDE.md hierarchy with child files

The current single-file CLAUDE.md works but will become unwieldy as the project grows. Best practice is to use child CLAUDE.md files in subdirectories for directory-scoped instructions:

```
CLAUDE.md                          # Core: build, test, versioned-build mandate (target: ~1,200 tokens)
Source/CLAUDE.md                   # Source-specific: processBlock rules, lock-free constraints, framework boundaries
docs/CLAUDE.md                     # Docs: version registry location, legal notice generation workflow
```

Claude loads child CLAUDE.md files only when working in that directory [Source 2]. This would further reduce the always-loaded budget.

**Risk:** Low, but adds maintenance surface. Recommend only after v1.0 ships.

### B. Periodic memory pruning protocol

Two project memories may be stale after v1.0 ships:
- `project_ship_date.md` — will need updating or removal after 2026-04-10
- `project_version_reset.md` — historical event, diminishing relevance

Establish a quarterly review: scan memory files, remove any that are stale or whose content is now in CLAUDE.md or the codebase. The auto-memory system instructions already advise this ("Update or remove memories that turn out to be wrong or outdated").

### C. Consider a project-level `.claude/skills/` cleanup strategy

After recommendation #2 (remove duplicates), the 6 project-only skills should be evaluated:
- `canvas-design`, `docx`, `pdf` — utility skills, not audio-specific. Consider whether these belong at user level instead.
- `doc-coauthoring` — workflow skill, potentially useful across projects.
- `requesting-code-review`, `systematic-debugging` — overlap with superpowers plugin versions.

---

## Open Questions for Human Review

1. **Skill consolidation scope (Rec #5):** The 13 potentially-irrelevant skills listed — do you use any of these in other projects that share this Claude Code installation? Disabling user-level skills is global.

2. **Notion integration consolidation (Rec #6):** Do you need both Notion integrations simultaneously, or could the `notion-sml` skill be repointed to use the personal integration?

3. **CLAUDE.md architecture notes (Rec #7):** The HRTF signal flow, PartitionedConvolver, Phase Vocoder, and Dry Path Compensation sections — do you want these in the always-loaded system prompt, or are you comfortable with Claude reading them from a memory file on demand when doing DSP work?

4. **Child CLAUDE.md adoption (Structural Rec A):** Do you want directory-scoped CLAUDE.md files now, or defer until after v1.0 ships?

5. **GSD plugin overhead:** The GSD plugin contributes 60 skills, 24 agent definitions (434KB on disk), and 9 hooks. While most of this is deferred/on-demand, the skill listing alone is ~3,000 tokens of GSD entries. Is GSD actively used for this project, or is it installed globally and mostly used elsewhere?

---

## Sources

1. **Anthropic — "Effective Context Engineering for AI Agents"** (2026)
   https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents
   Key insight: "The LLM is a CPU, the context window is RAM." Context rot degrades precision even within the window limit.

2. **Claude Code Docs — "Best Practices" / Writing a good CLAUDE.md** (2025-2026)
   https://code.claude.com/docs/en/best-practices
   https://www.humanlayer.dev/blog/writing-a-good-claude-md
   Key insight: Keep CLAUDE.md under ~500 tokens of instructions. Every line should pass "would removing this cause mistakes?"

3. **HumanLayer — "Skill Issue: Harness Engineering for Coding Agents"** (2026)
   https://www.humanlayer.dev/blog/skill-issue-harness-engineering-for-coding-agents
   Key insight: Skills implement progressive disclosure. ~20-50 tokens per skill for discovery. Full body loads only on activation.

4. **Breunig — "How Claude Code Builds a System Prompt"** (2026-04-04)
   https://www.dbreunig.com/2026/04/04/how-claude-code-builds-a-system-prompt.html
   Key insight: Cache boundary separates static from session-specific content. Smaller static content = better cache hit rates.

5. **OnlyCLI — "MCP Token Trap: Benchmark"** (2026)
   https://onlycli.github.io/OnlyCLI/blog/mcp-token-cost-benchmark/
   Key insight: MCP tools cost 550-1,400 tokens each. Keep active servers to 3-5. CLI tools use 4-32x fewer tokens.

6. **MindStudio — "Claude Code MCP Token Overhead"** (2026)
   https://www.mindstudio.ai/blog/claude-code-mcp-server-token-overhead-2
   Key insight: 3 MCP servers combined can consume 72% of a 200K window. Optimize schemas for 40-60% reduction.

7. **Apideck — "MCP Server Eating Your Context Window"** (2026)
   https://www.apideck.com/blog/mcp-server-eating-context-window-cli-alternative
   Key insight: Prefer CLI tools over MCP where possible — equivalent operations at fraction of token cost.
