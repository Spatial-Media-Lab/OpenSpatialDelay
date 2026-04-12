# Context Audit Report

**Date:** 2026-04-10  
**Project:** OpenSpatialDelay (`/Users/andrewrahman/conductor/repos/openspatialdelay`)  
**Model:** Claude Opus 4.6 (1M context)

---

## 1. Baseline Totals

> **Note:** The `/context` CLI command could not be executed programmatically (`claude --print-context` is not available at the detected path). All token estimates below are derived from measuring the injected content visible in the session's system-reminder messages and comparing against known byte-to-token ratios (~3.5–4.0 characters per token for English/Markdown).

### Estimated context consumption at session start

| Source | Bytes | Est. Tokens | Notes |
|--------|-------|-------------|-------|
| System prompt (core instructions) | ~12,000 | ~3,400 | Tool descriptions, behavior rules, tone guidelines |
| Agent tool description (29 agent types) | ~5,500 | ~1,600 | Listed inline in Agent tool schema |
| CLAUDE.md (project) | 7,445 | ~2,100 | Injected verbatim into `claudeMd` system-reminder |
| MEMORY.md (auto-memory index) | 11,850 | ~3,400 | Injected verbatim into `claudeMd` system-reminder |
| Skill listing (system-reminder) | ~18,000 (est.) | ~5,100 | 147 skill names + descriptions |
| Deferred tool listing (system-reminder) | ~3,500 (est.) | ~1,000 | 59 tool names (no schemas until fetched) |
| Superpowers `using-superpowers` skill (auto-loaded) | ~3,800 (est.) | ~1,100 | Full skill content injected at session start |
| $CMEM session hook output | ~600 (est.) | ~170 | claude-mem session state block |
| Git status snapshot | ~400 (est.) | ~115 | Branch, recent commits, status |
| Auto-memory system instructions | ~4,500 (est.) | ~1,300 | Memory types, rules, save/read protocol |
| Tool schemas (7 always-loaded tools) | ~6,500 (est.) | ~1,850 | Bash, Edit, Glob, Grep, Read, Skill, Write, Agent, ToolSearch |
| **TOTAL ESTIMATED** | **~74,095** | **~21,135** | |

**Model context window:** 1,000,000 tokens  
**Estimated free space at session start:** ~978,865 tokens (~97.9%)  
**Assumption:** Token estimates use ~3.5 chars/token. Actual tokenization varies. These are order-of-magnitude estimates.

---

## 2. CLAUDE.md Inventory

### 2.1 Project CLAUDE.md (only instance found)

- **Full path:** `/Users/andrewrahman/conductor/repos/openspatialdelay/CLAUDE.md`
- **Size:** 7,445 bytes
- **Estimated tokens:** ~2,100
- **Last modified:** Apr 7, 2026 15:43:13
- **Purpose:** Defines build commands, versioned build workflow, version registry, test infrastructure, architecture notes (signal flow, convolver, pitch shifter, dry path compensation), and licensing/monetization details.
- **Loaded at:** Every session start, injected into `claudeMd` system-reminder.

**Full verbatim contents:**

````markdown
# CLAUDE.md — OpenSpatialDelay

## Build & Test Rules

### Versioning: Major.Minor.Bugfix (issue #55)

OpenSpatialDelay uses **semantic versioning vX.Y.Z**:
- **X (Major):** Breaking changes, new architecture, incompatible preset format
- **Y (Minor):** New features, new output formats, new algorithms
- **Z (Bugfix):** Bug fixes, threshold tweaks, documentation updates

**Current release: v1.0.0** (pre-release, shipping end of week 2026-04-10)

All prior bugfix builds (v1.0.1–v1.0.2 for issues #76, #77) have been collapsed into v1.0.0. The next bugfix build will be v1.0.1.

### MANDATORY: Versioned builds for every fix attempt

Every code change that needs manual listening/testing MUST be built as a uniquely-named versioned plugin. **Never reuse a version number. Never skip this step.**

```bash
# 1. Commit your changes
git add ... && git commit -m "..."

# 2. Build as a versioned plugin (increment Z each time)
bash scripts/build_version.sh <commit-hash> v1.0.Z O10Z
```

**Why this is critical:** The default `cmake --build` installs to `OpenSpatialDelay v1.0.component`. The user tests with individually-named versioned plugins (e.g., `OpenSpatialDelay v1.0.1.component`) loaded side-by-side in REAPER for A/B comparison. If you don't use `build_version.sh`, the user will never hear your changes.

**Version number registry (do not reuse) — reset 2026-04-02, collapsed after issue #77:**
- v1.0.0 / O100 — baseline (commit 3ef9c79, includes issues #76 + #77)
- v1.0.1 / O101 — fix dry signal attenuation at 0% wet (issue #97, commit a0f39c5)
- v1.0.2 / O102 — call updateHostDisplay() on config changes (issue #94, commit dbf19ce)
- v1.0.3 / O103 — fix direction toggle for Bounce, Line, Random (issue #100, commit 9dd6266)
- v1.0.5 / O105 — reset phase vocoder on preset change chirp (issue #99, commit a9df52a)
- v1.0.6 / O106 — transport fade-in to prevent scrub/seek click (issue #103, commit 7901a0a)
- v1.0.8 / O108 — fix Ableton crash + automation reset (issue #122, commit 0d3cc53)
- v1.0.9 / O109 — re-sign bundles after plist patching for Ableton VST3 visibility (issue #120)
- v1.0.10 / O110 — reduce automatable params to 64 for Ableton auto-populate (issue #122, commit 4adfd1d)
- v1.0.7 / O107 — 4-layer tape wobble emulation (issue #92, commit 893acb2)
- v1.0.14 / O114 — shared LookAndFeel + visibility throttle for multi-instance crash (issue #131)
- v1.0.17 / O117 — shared FFT cache for multi-instance vDSP stability (issue #131, commit e8ee6ec)
- v1.0.20 / O120 — default Input to Stereo on stereo tracks, disable on mono (issue #155, commit 376b5fe)
- Next available: **v1.0.21 / O121**

### Running tests

```bash
# Configure (first time or after clean)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev

# Build and run tests
cmake --build build --target OpenSpatialDelayTests -j$(sysctl -n hw.ncpu)
./build/OpenSpatialDelayTests

# HRTF tests require synchronous profile loading (testLoadHRTFProfile)
# The timer thread doesn't fire in the test harness
```

### Test infrastructure notes

- All HRTF binaural tests use `testLoadHRTFProfile()` for synchronous HRTF loading
- Without this, the timer-based loading never fires and tests run in Simple mode (false positives)
- The `createBinauralProcessor()` helper handles this automatically

## Architecture Notes

### HRTF rendering signal flow
1. `renderDirectBinauralHRTF()` — 3-pass architecture
2. Pass 1: per-sample delay engine → per-source mono accumulation
3. Pass 2: per-block HRTF convolution via `BinauralRenderer::renderSourceBuffers()`
4. Pass 3: per-sample dry/wet mix + output gain

### PartitionedConvolver
Uses spectral envelope EMA smoothing: magnitude and phase smoothed separately per frequency bin. This prevents comb filtering from phase-misaligned time-domain blending.

### Phase Vocoder Pitch Shifter
Replaces WSOLA-Lite. Uses STFT (2048-point FFT, 4x overlap) with:
- Laroche-Dolson phase locking for tonal content
- Röbel-style spectral flux transient detection with adaptive median threshold
- Phase reset on transient frames preserves attack sharpness
- Latency: 2048 samples (reported to DAW via setLatencySamples)
- Range: ±12 semitones (combined with Doppler)

### Dry Path Latency Compensation (v1.0.7) + Stereo Dry (v1.0.1)
The phase vocoder adds 2048 samples of latency to the wet path. The dry signal must be delayed by the same amount so the DAW's plugin delay compensation (PDC) is correct at all dry/wet settings. Implemented as stereo circular `dryDelayLineL/R` buffers that pre-fill `dryCompBufferL/R` from raw DAW input at the start of each processBlock.

The dry/wet mix happens in a single post-render stage in processBlock — render paths output raw wet signal only. This ensures the dry signal truly bypasses the entire plugin (Input selector only affects the wet path). Equal-power crossfade (cos/sin) replaces linear (1-dw/dw) for constant perceived loudness at all mix settings.

### ITD delay line
For MIT KEMAR SOFA file, ITD values are always 0 (embedded in HRIR waveform). The ITD delay line is effectively a pass-through for this dataset.

## Licensing & Monetization

### Dual-license model
OpenSpatialDelay is dual-licensed: **GPL-3.0** (free/open-source) and **Commercial License** (proprietary/closed-source, via Spatial Media Lab). Both paths must be compatible with every distributed third-party component.

### JUCE 8 commercial tiers (as of April 2026)

| Tier | Revenue Cap | Cost | Closed-Source OK |
|------|-------------|------|------------------|
| Starter | $20,000/year | Free | Yes |
| Indie | $300,000/year | $40/mo or $800 perpetual | Yes |
| Pro | No limit | $175/mo or $3,500 perpetual | Yes |

**Starter tier allows free commercial distribution** with no splash screen and no attribution required, as long as total entity revenue stays under $20K/year. Upgrade to Indie at $20K+, Pro at $300K+.

Source: [JUCE 8 EULA](https://juce.com/legal/juce-8-licence/) and [juce.com/get-juce](https://juce.com/get-juce/)

### Third-party license compatibility (verified 2026-04-05)

All distributed third-party components are compatible with both GPL-3.0 and commercial licensing:

| Component | License | Commercial Use | Obligation |
|-----------|---------|----------------|------------|
| JUCE 8 | GPL-3.0 / Commercial | Yes (Starter tier free < $20K) | Upgrade tier at revenue thresholds |
| libmysofa v1.3.2 | BSD-3-Clause | Yes | Include license notice |
| zlib | zlib License | Yes | Include license notice |
| MIT KEMAR (Studio Reference) | MIT | Yes | Include copyright + permission notice |
| SADIE II D2 KU100 (Immersive) | Apache 2.0 | Yes | Include license notice |
| CIPIC Subject 003 (Natural) | Public Domain | Yes | None |
| HUTUBS PP2 (Precise) | CC BY 4.0 | Yes | Attribution required |
| Bernschuetz KU100 (Spatial) | CC BY 3.0 | Yes | Attribution required |
| DM Sans | SIL OFL 1.1 | Yes | Cannot sell font standalone |
| JetBrains Mono | SIL OFL 1.1 | Yes | Cannot sell font standalone |
| Roboto | Apache 2.0 | Yes | Include license notice |

All attribution obligations are fulfilled by the legal notices document at `docs/OpenSpatialDelay_Legal_Notices.docx` (generated by `docs/generate_legal_notices.js`) and the Third-Party Notices chapter in the user manual.
````

### 2.2 No other CLAUDE.md files found

- No global `~/.claude/CLAUDE.md`
- No `~/CLAUDE.md`
- No subdirectory CLAUDE.md files within the project
- Glob for `**/CLAUDE.md` returned only the project root instance

---

## 3. MCP Servers

### 3.1 Claude AI Notion (built-in integration)

- **Name:** `claude.ai Notion`
- **Type:** Built-in Claude AI integration (personal Notion workspace)
- **Tools exposed:** 16
- **Token cost:** ~0 at session start (all 16 tools are deferred — only names listed, schemas loaded on demand via ToolSearch)
- **Tool names:**
  1. `mcp__claude_ai_Notion__notion-create-comment`
  2. `mcp__claude_ai_Notion__notion-create-database`
  3. `mcp__claude_ai_Notion__notion-create-pages`
  4. `mcp__claude_ai_Notion__notion-create-view`
  5. `mcp__claude_ai_Notion__notion-duplicate-page`
  6. `mcp__claude_ai_Notion__notion-fetch`
  7. `mcp__claude_ai_Notion__notion-get-comments`
  8. `mcp__claude_ai_Notion__notion-get-teams`
  9. `mcp__claude_ai_Notion__notion-get-users`
  10. `mcp__claude_ai_Notion__notion-move-pages`
  11. `mcp__claude_ai_Notion__notion-search`
  12. `mcp__claude_ai_Notion__notion-update-data-source`
  13. `mcp__claude_ai_Notion__notion-update-page`
  14. `mcp__claude_ai_Notion__notion-update-view`
  15. (Note: 16 listed in system-reminder — 2 may be hidden or the count is from the tool listing)

### 3.2 Notion SML (custom MCP integration)

- **Name:** `notion-sml`
- **Type:** Custom MCP server (likely via plugin or skill)
- **Tools exposed:** 21
- **Token cost:** ~0 at session start (all deferred)
- **Tool names:**
  1. `mcp__notion-sml__API-create-a-comment`
  2. `mcp__notion-sml__API-create-a-data-source`
  3. `mcp__notion-sml__API-delete-a-block`
  4. `mcp__notion-sml__API-get-block-children`
  5. `mcp__notion-sml__API-get-self`
  6. `mcp__notion-sml__API-get-user`
  7. `mcp__notion-sml__API-get-users`
  8. `mcp__notion-sml__API-list-data-source-templates`
  9. `mcp__notion-sml__API-move-page`
  10. `mcp__notion-sml__API-patch-block-children`
  11. `mcp__notion-sml__API-patch-page`
  12. `mcp__notion-sml__API-post-page`
  13. `mcp__notion-sml__API-post-search`
  14. `mcp__notion-sml__API-query-data-source`
  15. `mcp__notion-sml__API-retrieve-a-block`
  16. `mcp__notion-sml__API-retrieve-a-comment`
  17. `mcp__notion-sml__API-retrieve-a-data-source`
  18. `mcp__notion-sml__API-retrieve-a-database`
  19. `mcp__notion-sml__API-retrieve-a-page`
  20. `mcp__notion-sml__API-retrieve-a-page-property`
  21. `mcp__notion-sml__API-update-a-block`
  22. `mcp__notion-sml__API-update-a-data-source`

### 3.3 Claude AI Gmail (built-in, needs auth)

- **Name:** `claude.ai Gmail`
- **Tools exposed:** 1 (`mcp__claude_ai_Gmail__authenticate`)
- **Token cost:** ~0 (deferred)
- **Auth status:** Needs authentication (cached as needing auth at timestamp 1775845804811)

### 3.4 Claude AI Google Calendar (built-in, needs auth)

- **Name:** `claude.ai Google Calendar`
- **Tools exposed:** 1 (`mcp__claude_ai_Google_Calendar__authenticate`)
- **Token cost:** ~0 (deferred)
- **Auth status:** Needs authentication (cached as needing auth at timestamp 1775845804813)

### 3.5 MCP Configuration Files

- **No project-level `.mcp.json`** found at project root
- **No global `~/.claude/mcp.json`** found
- MCP servers appear to be configured via plugins and built-in Claude AI integrations, not via JSON config files

---

## 4. Skills / Plugins / Auto-loaded Files

### 4.1 Enabled Plugins (from `settings.json`)

| Plugin | Version | Source | Status |
|--------|---------|--------|--------|
| superpowers@claude-plugins-official | 5.0.7 | anthropics/claude-plugins-official | **Enabled** |
| typescript-lsp@claude-plugins-official | 1.0.0 | anthropics/claude-plugins-official | **Enabled** |
| frontend-design@claude-plugins-official | unknown | anthropics/claude-plugins-official | **Enabled** |
| claude-mem@thedotmack | 12.0.0 | thedotmack/claude-mem | **Enabled** |

### 4.2 Installed but NOT Enabled Plugins

| Plugin | Version | Source |
|--------|---------|--------|
| code-review@claude-plugins-official | unknown | anthropics/claude-plugins-official |
| github@claude-plugins-official | unknown | anthropics/claude-plugins-official |
| feature-dev@claude-plugins-official | unknown | anthropics/claude-plugins-official |
| code-simplifier@claude-plugins-official | 1.0.0 | anthropics/claude-plugins-official |
| claude-md-management@claude-plugins-official | 1.0.0 | anthropics/claude-plugins-official |
| context7@claude-plugins-official | unknown | anthropics/claude-plugins-official |
| greptile@claude-plugins-official | unknown | anthropics/claude-plugins-official |

### 4.3 Skills Inventory

Skills are **not loaded at session start** — only their names and one-line descriptions appear in the skill listing system-reminder. Full skill content is loaded on demand when invoked via the Skill tool.

#### Skill counts by location

| Location | Count | Disk Size (all SKILL.md) | Session-start cost |
|----------|-------|--------------------------|-------------------|
| User-level (`~/.claude/skills/`) | 109 directories (107 with SKILL.md) | 441,849 bytes | Names+descriptions only (~18KB est.) |
| Project-level (`.claude/skills/`) | 22 directories | 315,773 bytes (deduped) | Names+descriptions only (included in above) |
| Superpowers plugin skills | ~15 skills | 245,122 bytes | Names+descriptions only |
| claude-mem plugin skills | ~6 skills | ~30,000 bytes (est.) | Names+descriptions only |
| frontend-design plugin skills | ~1 skill | 4,274 bytes | Names+descriptions only |
| **Total unique skill names listed** | **~147** | **~1,037,018 bytes on disk** | **~18,000 bytes (~5,100 tokens) in listing** |

#### User-level skill categories (109 directories in `~/.claude/skills/`)

- **GSD (Get Shit Done) workflow:** 60 skills (gsd-*)
- **Audio/JUCE domain:** 9 skills (sound-engineer, spatial-audio-dsp, dsp-cookbook, juce-best-practices, reverb-algorithms, synthesis-techniques, time-based-effects, cross-platform-builds, daw-compatibility-guide, plugin-architecture-patterns)
- **Code quality:** 4 skills (code-quality-checker, dead-code-auditor, runtime-performance-auditor, improve-codebase-architecture)
- **Development workflow:** 12 skills (pair-programming, skill-builder, triage-issue, request-refactor-plan, prd-to-plan, prd-to-issues, write-a-prd, design-an-interface, scaffold-exercises, setup-pre-commit, git-guardrails-claude-code, migrate-to-shoehorn)
- **Content/docs:** 5 skills (edit-article, deep-research, last30days, grill-me, rubber-duck)
- **UI/design:** 3 skills (brand-forge, macos-design, oiloil-ui-ux-guide)
- **Integrations:** 3 skills (notion-sml, obsidian-vault, make-scenario-builder)
- **Other:** 5 skills (dual-mode, ubiquitous-language, pages, make-automation, doc-coauthoring)

#### Project-level skills (22 directories in `.claude/skills/`)

canvas-design, code-quality-checker, cross-platform-builds, daw-compatibility-guide, dead-code-auditor, doc-coauthoring, docx, dsp-cookbook, juce-best-practices, macos-design, oiloil-ui-ux-guide, pdf, plugin-architecture-patterns, requesting-code-review, reverb-algorithms, runtime-performance-auditor, sound-engineer, spatial-audio-dsp, synthesis-techniques, systematic-debugging, time-based-effects, verification-before-completion

### 4.4 Agent Definitions (`~/.claude/agents/`)

Agent definitions are loaded as part of the Agent tool's type listing (names + descriptions only). Full definition content is loaded when an agent is dispatched.

| Agent Definition | Size (bytes) |
|-----------------|-------------|
| gsd-planner.md | 45,980 |
| gsd-debugger.md | 43,120 |
| gsd-doc-writer.md | 37,136 |
| gsd-plan-checker.md | 30,188 |
| gsd-verifier.md | 30,176 |
| gsd-phase-researcher.md | 28,576 |
| gsd-executor.md | 21,963 |
| gsd-code-fixer.md | 19,873 |
| gsd-roadmapper.md | 18,359 |
| gsd-project-researcher.md | 17,512 |
| gsd-codebase-mapper.md | 16,515 |
| gsd-ui-auditor.md | 15,976 |
| gsd-code-reviewer.md | 14,093 |
| gsd-integration-checker.md | 13,216 |
| gsd-ui-researcher.md | 13,029 |
| gsd-doc-verifier.md | 11,099 |
| gsd-intel-updater.md | 11,114 |
| gsd-ui-checker.md | 10,394 |
| gsd-user-profiler.md | 8,535 |
| gsd-research-synthesizer.md | 7,252 |
| gsd-nyquist-auditor.md | 5,251 |
| gsd-advisor-researcher.md | 4,424 |
| gsd-assumptions-analyzer.md | 4,489 |
| gsd-security-auditor.md | 4,340 |
| typescript-specialist.md | 332 |
| security-auditor.md | 336 |
| database-specialist.md | 328 |
| python-specialist.md | 297 |
| project-coordinator.md | 173 |
| **TOTAL** | **434,076 bytes (~124K tokens on disk, but only ~1,600 tokens of names/descriptions loaded at session start)** |

### 4.5 Hooks (execute at runtime, not loaded into context)

Hooks are shell scripts/Node.js files that execute at specific events. They do NOT consume context tokens directly, but their **output** can be injected into context.

| Hook | File | Size | Trigger |
|------|------|------|---------|
| gsd-check-update.js | `~/.claude/hooks/` | 5,368 B | SessionStart |
| gsd-session-state.sh | `~/.claude/hooks/` | 1,090 B | SessionStart |
| gsd-context-monitor.js | `~/.claude/hooks/` | 6,362 B | PostToolUse (Bash\|Edit\|Write\|MultiEdit\|Agent\|Task) |
| gsd-phase-boundary.sh | `~/.claude/hooks/` | 1,197 B | PostToolUse (Write\|Edit) |
| gsd-prompt-guard.js | `~/.claude/hooks/` | 3,490 B | PreToolUse (Write\|Edit) |
| gsd-read-guard.js | `~/.claude/hooks/` | 2,675 B | PreToolUse (Write\|Edit) |
| gsd-workflow-guard.js | `~/.claude/hooks/` | 3,353 B | PreToolUse (Write\|Edit) |
| gsd-validate-commit.sh | `~/.claude/hooks/` | 2,059 B | PreToolUse (Bash) |
| gsd-statusline.js | `~/.claude/hooks/` | 5,274 B | statusLine (continuous) |
| **TOTAL** | | **30,868 B** | |

**Session start hook output observed:** The `$CMEM` block (~600 bytes) and a `{"continue":true,"suppressOutput":true}` status line were injected at session start.

### 4.6 Auto-Memory System

The auto-memory system (`autoMemoryEnabled: true` in settings.json) loads `MEMORY.md` into context at every session start. Individual memory files are read on demand.

| File | Size (bytes) | Loaded at start? |
|------|-------------|-----------------|
| MEMORY.md (index) | 11,850 | **Yes** — always |
| feedback_versioned_builds.md | 1,356 | No — on demand |
| feedback_no_au_cache_refresh.md | 715 | No — on demand |
| feedback_version_discipline.md | 1,433 | No — on demand |
| feedback_version_comparison.md | 697 | No — on demand |
| feedback_no_dual_head_pitch.md | 838 | No — on demand |
| feedback_hrtf_profile_names.md | 840 | No — on demand |
| feedback_push_documentation.md | 854 | No — on demand |
| project_version_reset.md | 590 | No — on demand |
| project_ship_date.md | 504 | No — on demand |
| reference_notion_workspace.md | 530 | No — on demand |
| reference_notion_release_tests_db.md | 1,059 | No — on demand |
| reference_notion_doc_review_db.md | 1,014 | No — on demand |
| **TOTAL** | **22,280** | **11,850 at start** |

### 4.7 Other Settings

From `~/.claude/settings.json`:
- `effortLevel`: "high"
- `autoMemoryEnabled`: true
- `autoDreamEnabled`: true

From `.claude/settings.local.json` (project-level):
- 2 permissions allowed: `Bash(/opt/homebrew/bin/claude --print-context)` and `Bash(ls:*)`

---

## 5. Project Knowledge / Attached Files

### 5.1 Files referenced in context but NOT loaded at start

| File | Size (bytes) | Est. Tokens | Loaded? |
|------|-------------|-------------|---------|
| SPECIFICATION.md | 57,734 | ~16,500 | **No** — untracked in git, not auto-loaded |
| Source/PluginProcessor.cpp | ~175,000 (per MEMORY.md) | ~50,000 | No — read on demand |
| Source/PluginProcessor.h | ~33,000 (per MEMORY.md) | ~9,400 | No — read on demand |
| Source/PluginEditor.cpp | ~45,000 (per MEMORY.md) | ~12,900 | No — read on demand |
| Source/PluginEditor.h | ~14,000 (per MEMORY.md) | ~4,000 | No — read on demand |
| Source/PresetData.cpp | ~45,000 (per MEMORY.md) | ~12,900 | No — read on demand |

### 5.2 Other project files on disk

- `gsd-file-manifest.json`: 30,667 bytes — GSD plugin's file tracking manifest
- `HRTF/` directory: 5 SOFA files tracked via Git LFS (binary, not readable as context)
- `Archive/v0.1` through `v0.9`: frozen source snapshots (not loaded)

---

## 6. Observed Redundancies

### 6.1 CLAUDE.md ↔ MEMORY.md content overlap

The following information appears in both `CLAUDE.md` and `MEMORY.md`:

1. **Build system instructions** — CLAUDE.md has build commands; MEMORY.md §"Build System" repeats the same cmake commands, paths, and dependencies.
2. **Architecture / signal flow** — CLAUDE.md §"Architecture Notes" describes HRTF rendering, PartitionedConvolver, Phase Vocoder, Dry Path Compensation. MEMORY.md §"Architecture — Signal Flow (5 Stages)" and §"Key Architecture Details" cover the same information with different wording.
3. **Version registry** — CLAUDE.md has the full version number registry. MEMORY.md §"Version History Summary" covers the same ground at a higher level.
4. **HRTF profiles** — CLAUDE.md does not list them, but MEMORY.md §"HRTF Profiles" lists all 6. The feedback memory `feedback_hrtf_profile_names.md` also contains the full profile mapping table.
5. **Test infrastructure** — CLAUDE.md has test commands and notes. MEMORY.md §"Current Status" mentions the test count.
6. **Licensing tables** — CLAUDE.md has the full JUCE tier table and third-party license table. MEMORY.md does not duplicate this.

### 6.2 MEMORY.md ↔ individual memory files

MEMORY.md contains a §"Memories" section that lists all 12 individual memory files with one-line descriptions. This is an index, not a duplication of content — the individual files contain the full details. This is the intended design of the auto-memory system.

### 6.3 Skill duplication across locations

22 skills exist in both user-level (`~/.claude/skills/`) and project-level (`.claude/skills/`). These appear to be identical copies:

| Skill name | User-level | Project-level |
|-----------|-----------|--------------|
| code-quality-checker | Yes | Yes |
| cross-platform-builds | Yes | Yes |
| daw-compatibility-guide | Yes | Yes |
| dead-code-auditor | Yes | Yes |
| dsp-cookbook | Yes | Yes |
| juce-best-practices | Yes | Yes |
| macos-design | Yes | Yes |
| oiloil-ui-ux-guide | Yes | Yes |
| plugin-architecture-patterns | Yes | Yes |
| reverb-algorithms | Yes | Yes |
| runtime-performance-auditor | Yes | Yes |
| sound-engineer | Yes | Yes |
| spatial-audio-dsp | Yes | Yes |
| synthesis-techniques | Yes | Yes |
| time-based-effects | Yes | Yes |
| verification-before-completion | Yes | Yes |

Additionally, 6 skills exist **only** at project level (not duplicated at user level): canvas-design, doc-coauthoring, docx, pdf, requesting-code-review, systematic-debugging.

Some skills from plugins (superpowers) also duplicate user-level and project-level skills by name (e.g., `verification-before-completion`, `requesting-code-review`, `systematic-debugging`). The skill listing in the system-reminder shows both the plugin version and the local version as separate entries with distinct qualified names.

### 6.4 Deferred tools: dual Notion integrations

Two separate Notion MCP integrations expose overlapping functionality:
- `claude.ai Notion` (16 tools) — personal Notion integration
- `notion-sml` (22 tools) — SML-specific Notion integration

Both can search, fetch, create, and update pages. The `notion-sml` integration has additional tools (delete block, list data source templates, etc.).

### 6.5 Installed but unused plugins

7 plugins are installed but NOT enabled in `settings.json`. They consume disk space (~various) but do not contribute to context at session start. Their skills do NOT appear in the skill listing.

---

## 7. Raw Data Appendix

### 7.1 Full settings.json

```json
{
  "hooks": {
    "SessionStart": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "node \"/Users/andrewrahman/.claude/hooks/gsd-check-update.js\""
          }
        ]
      },
      {
        "hooks": [
          {
            "type": "command",
            "command": "bash /Users/andrewrahman/.claude/hooks/gsd-session-state.sh"
          }
        ]
      }
    ],
    "PostToolUse": [
      {
        "matcher": "Bash|Edit|Write|MultiEdit|Agent|Task",
        "hooks": [
          {
            "type": "command",
            "command": "node \"/Users/andrewrahman/.claude/hooks/gsd-context-monitor.js\"",
            "timeout": 10
          }
        ]
      },
      {
        "matcher": "Write|Edit",
        "hooks": [
          {
            "type": "command",
            "command": "bash /Users/andrewrahman/.claude/hooks/gsd-phase-boundary.sh",
            "timeout": 5
          }
        ]
      }
    ],
    "PreToolUse": [
      {
        "matcher": "Write|Edit",
        "hooks": [
          {
            "type": "command",
            "command": "node \"/Users/andrewrahman/.claude/hooks/gsd-prompt-guard.js\"",
            "timeout": 5
          }
        ]
      },
      {
        "matcher": "Write|Edit",
        "hooks": [
          {
            "type": "command",
            "command": "node \"/Users/andrewrahman/.claude/hooks/gsd-read-guard.js\"",
            "timeout": 5
          }
        ]
      },
      {
        "matcher": "Write|Edit",
        "hooks": [
          {
            "type": "command",
            "command": "node \"/Users/andrewrahman/.claude/hooks/gsd-workflow-guard.js\"",
            "timeout": 5
          }
        ]
      },
      {
        "matcher": "Bash",
        "hooks": [
          {
            "type": "command",
            "command": "bash /Users/andrewrahman/.claude/hooks/gsd-validate-commit.sh",
            "timeout": 5
          }
        ]
      }
    ]
  },
  "statusLine": {
    "type": "command",
    "command": "node \"/Users/andrewrahman/.claude/hooks/gsd-statusline.js\""
  },
  "enabledPlugins": {
    "superpowers@claude-plugins-official": true,
    "typescript-lsp@claude-plugins-official": true,
    "frontend-design@claude-plugins-official": true,
    "claude-mem@thedotmack": true
  },
  "extraKnownMarketplaces": {
    "claude-plugins-official": {
      "source": {
        "source": "github",
        "repo": "anthropics/claude-plugins-official"
      }
    },
    "thedotmack": {
      "source": {
        "source": "github",
        "repo": "thedotmack/claude-mem"
      }
    }
  },
  "effortLevel": "high",
  "autoMemoryEnabled": true,
  "autoDreamEnabled": true
}
```

### 7.2 Project-level .claude/settings.local.json

```json
{
  "permissions": {
    "allow": [
      "Bash(/opt/homebrew/bin/claude --print-context)",
      "Bash(ls:*)"
    ]
  }
}
```

### 7.3 Project-level .claude/launch.json

```json
{
  "version": "0.0.1",
  "configurations": [
    {
      "name": "manual-preview",
      "runtimeExecutable": "python3",
      "runtimeArgs": ["-m", "http.server", "8095", "--directory", "docs"],
      "port": 8095
    },
    {
      "name": "prototype",
      "runtimeExecutable": "python3",
      "runtimeArgs": ["-m", "http.server", "8090", "--directory", "docs/design-prototypes"],
      "port": 8090
    }
  ]
}
```

### 7.4 Deferred tools (complete list — 59 tools)

**Core (20):** AskUserQuestion, CronCreate, CronDelete, CronList, EnterPlanMode, EnterWorktree, ExitPlanMode, ExitWorktree, ListMcpResourcesTool, NotebookEdit, ReadMcpResourceTool, RemoteTrigger, TaskCreate, TaskGet, TaskList, TaskOutput, TaskStop, TaskUpdate, WebFetch, WebSearch

**Claude AI Gmail (1):** mcp__claude_ai_Gmail__authenticate

**Claude AI Google Calendar (1):** mcp__claude_ai_Google_Calendar__authenticate

**Claude AI Notion (16):** mcp__claude_ai_Notion__notion-create-comment, notion-create-database, notion-create-pages, notion-create-view, notion-duplicate-page, notion-fetch, notion-get-comments, notion-get-teams, notion-get-users, notion-move-pages, notion-search, notion-update-data-source, notion-update-page, notion-update-view (14 unique listed — 2 may be unlisted or the count includes internal tools)

**Notion SML (21):** mcp__notion-sml__API-create-a-comment, API-create-a-data-source, API-delete-a-block, API-get-block-children, API-get-self, API-get-user, API-get-users, API-list-data-source-templates, API-move-page, API-patch-block-children, API-patch-page, API-post-page, API-post-search, API-query-data-source, API-retrieve-a-block, API-retrieve-a-comment, API-retrieve-a-data-source, API-retrieve-a-database, API-retrieve-a-page, API-retrieve-a-page-property, API-update-a-block, API-update-a-data-source

### 7.5 Memory file contents (all 12 individual files)

#### feedback_hrtf_profile_names.md (840 bytes)
```markdown
---
name: Use user-facing HRTF profile names
description: Always refer to HRTF profiles by their UI names, not dataset names
type: feedback
---

Always use the user-facing HRTF profile names when communicating, not the internal dataset names.

**Why:** The user sees only the UI names in the plugin dropdown.

**How to apply:** Use mapping: 0=Simple (Low CPU), 1=Studio Reference, 2=Immersive, 3=Natural, 4=Precise, 5=Spatial
```

#### feedback_no_au_cache_refresh.md (715 bytes)
```markdown
---
name: Never refresh AU cache
description: NEVER run killall AudioComponentRegistrar or auval -a
type: feedback
---

NEVER run `killall AudioComponentRegistrar`, `auval -a`, or any other command to refresh/reset the macOS Audio Unit cache.

**Why:** User has explicitly told us not to do this multiple times. It's disruptive and unnecessary.

**How to apply:** After building with build_version.sh, trust the script's validation output.
```

#### feedback_no_dual_head_pitch.md (838 bytes)
```markdown
---
name: No dual-head pitch shifter
description: Dual-head crossfade pitch shifter sounds choppy on transients — rejected in favor of WSOLA-Lite
type: feedback
---

Never replace the WSOLA-Lite pitch shifter with dual-head crossfade. The dual-head approach was abandoned because it sounds choppy on transients (snares, kicks).

**Why:** WSOLA-Lite chosen for superior transient handling.

**How to apply:** When pitch shifter issues arise, fix the WSOLA-Lite implementation rather than switching algorithms.
```

#### feedback_push_documentation.md (854 bytes)
```markdown
---
name: Document pushes to remote
description: Always provide documentation when pushing to the remote
type: feedback
---

When pushing to remote, always provide documentation about what is being pushed.

**Why:** User expects a paper trail.

**How to apply:** Update relevant GitHub issue with comment, or include detailed PR description. Never just `git push` without documentation.
```

#### feedback_version_comparison.md (697 bytes)
```markdown
---
name: Version comparison semantics
description: v1.0 is NOT less than v1.0.0 — never treat shortened version strings as lesser versions
type: feedback
---

"v1.0" is NOT less than "v1.0.0". They represent the same major.minor version.

**Why:** User was furious when asked if v1.0 should be removed alongside v0.x versions.

**How to apply:** Treat shortened forms (v1.0) as equivalent to their full form (v1.0.0).
```

#### feedback_version_discipline.md (1,433 bytes)
```markdown
---
name: Version discipline for issue fixes
description: Stay on the same version number across all iterations of a single issue fix
type: feedback
---

Stay on the same version number for every build attempt of a single fix. Only increment when (a) current issue confirmed resolved AND (b) starting next issue.

**Why:** User A/B tests versioned builds side-by-side. Multiple half-fixed builds create confusion. Version-number space gets wasted.

**How to apply:** During fix cycle: rebuild same version. After user confirms: bump to next. Never bump mid-debug.
```

#### feedback_versioned_builds.md (1,356 bytes)
```markdown
---
name: Always use build_version.sh for testable builds
description: Every code change must be built as a uniquely-named versioned plugin
type: feedback
---

Always build using `scripts/build_version.sh <commit> <version> <plugin_code>`.

**Why:** 10 fix attempts were wasted because user was loading old versioned plugins while new code went to unversioned binary.

**How to apply:** Commit → build_version.sh → user tests in REAPER. Stay on same version number across iterations. Never overwrite unversioned binary accidentally.
```

#### project_ship_date.md (504 bytes)
```markdown
---
name: v1.0 ship date
description: v1.0.0 has NOT shipped yet — target is end of week 2026-04-10
type: project
---

v1.0.0 has not shipped. Target ship date is end of week 2026-04-10.

**Why:** CLAUDE.md previously had incorrect wording suggesting it had shipped.

**How to apply:** No migration concerns for format changes — no users have saved presets yet.
```

#### project_version_reset.md (590 bytes)
```markdown
---
name: Version registry reset to v1.0.0
description: All versions collapsed to v1.0.0 baseline as of 2026-04-01
type: project
---

Version registry was reset on 2026-04-01. All previous versions collapsed into v1.0.0 baseline.

**Why:** Consolidation of all prior fixes.

**How to apply:** Historical reset event. See feedback_version_discipline.md for live policy.
```

#### reference_notion_workspace.md (530 bytes)
```markdown
---
name: Notion workspace location
description: Create Notion pages under "Claude's space" parent page
type: reference
---

Create Notion pages/databases under the "Claude's space" page, not at workspace root.
```

#### reference_notion_release_tests_db.md (1,059 bytes)
```markdown
---
name: OSD v1.0 Release Tests NotionDB
description: Location of the OSD bug/test tracking database in Notion
type: reference
---

DB name: OSD v1.0 Release Tests
DB ID: d6ad01a1-f5b5-419e-8749-0f9b1dbc4d4a
Data source ID: collection://431dbd2a-ace0-4727-a5a7-039567a4a67f
Access via: Personal Notion integration (mcp__claude_ai_Notion__)
Key properties: Test ID, Test (title), Section, Status, Build, GitHub Issue, Notes
```

#### reference_notion_doc_review_db.md (1,014 bytes)
```markdown
---
name: OSD Documentation Review NotionDB
description: Location of the documentation issue tracking database
type: reference
---

DB name: OSD Documentation Review
DB ID: 6c015fd8-337c-4d45-b104-af3826969d88
Access via: Personal Notion integration
Issue ID format: DOC-xxx
```

### 7.6 Assumptions made during this audit

1. **Token estimation ratio:** Used ~3.5 characters per token. Actual BPE tokenization varies by content type (code vs. prose vs. structured data).
2. **Skill listing size:** Estimated at ~18KB based on 147 skills × ~120 bytes avg per entry (name + description line). Not directly measurable.
3. **System prompt size:** Estimated at ~12KB for core instructions. The actual system prompt is not directly readable as a file.
4. **"Loaded at session start":** Determined by observing what appears in system-reminder tags in the initial conversation context. Skills, agent definitions, and deferred tool schemas are NOT loaded until invoked.
5. **Plugin skill counts:** Based on the skill listing in the system-reminder, not direct file enumeration of all plugin skill directories.

### 7.7 Disk space summary

| Category | Total Bytes | Description |
|----------|-------------|-------------|
| CLAUDE.md | 7,445 | Project instructions |
| Memory system | 22,280 | MEMORY.md + 12 individual memory files |
| Agent definitions | 434,076 | 29 agent .md files |
| User-level skills (SKILL.md only) | 441,849 | 107 skill definitions |
| Project-level skills (SKILL.md only) | 315,773 | 22 skill definitions (deduped estimate) |
| Superpowers plugin skills | 245,122 | ~37 .md files |
| Hooks | 30,868 | 9 hook scripts |
| GSD file manifest | 30,667 | Plugin tracking |
| SPECIFICATION.md | 57,734 | Full project spec (untracked) |
| **TOTAL on-disk context-adjacent** | **~1,585,814** | **~1.5 MB** |
