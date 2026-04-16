---
phase: 03-personal-website
type: discovery
status: open
date_captured: 2026-04-16
captured_during: pre-03-discuss
sources:
  - Source/PluginEditor.cpp (plugin)
  - ../andrewrahman-com/app/globals.css (site)
  - docs/phase-02-evidence/patreon-graphics/_generate.py (Patreon assets)
---

# Phase 3 — Palette Drift Audit

Starting-point data for the Phase 3 discussion. User flagged that the colour
scheme has drifted across the plugin, website, and Patreon graphics. This
audit grep'd all three surfaces to quantify the drift before we commit to a
canonical style guide.

## Core finding

The **12-tap rainbow (tap-1 … tap-12) is identical across all three
surfaces** — that's the non-negotiable, already-canonical palette. Drift
exists only in the *supporting* tokens (backgrounds, text, secondary
accents), where each surface invented slightly different greys/accents
because no single source-of-truth existed when each was authored.

## Drift table

| Token | Plugin (`PluginEditor.cpp`) | Site (`globals.css`) | Patreon generator (`_generate.py`) |
|-------|------------------------------|----------------------|-------------------------------------|
| text-primary    | —           | `#e1e5ea`   | `#f0f4fa` (inlined) |
| text-secondary  | `#9fa2b0`   | `#9fa5ae`   | `#a0a8bc` (inlined) |
| text-dim        | `#6d7080`   | `#6d7279`   | —                   |
| void-soft       | —           | —           | `#0c101a` (invented) |
| accent-stellar  | `#7cc8f0` (internal highlight) | `#80d8ff` | — |
| accent-sync     | —           | `#e1c34b`   | —                   |
| accent-channel-l | —          | `#4499ff`   | —                   |
| accent-channel-r | —          | `#ff4444`   | —                   |

Plus: plugin uses `oklch`-derived variants (`#e0f0fa`, `#f0d8a8`, `#c0a8f0`)
for specific readouts that have no named token anywhere else.

## Characterisation

- **~80% aligned** — bg-void/bg-panel/border/12-tap colours all match.
- **Three layers each invented slightly different greys** where no canonical
  token existed — text-primary/secondary/dim drift by 3–15 hex units.
- **Site introduced 4 tokens** not present in the plugin: `stellar`, `sync`,
  `channel-l`, `channel-r`.
- **Patreon generator inlined RGB tuples** instead of referencing any
  shared palette — easiest surface to retrofit once a canonical file exists.

## Open decisions (for Phase 3 discussion)

1. **Where does the style guide live?**
   - Option A — this repo (`docs/DESIGN-SYSTEM.md` + `docs/design-tokens.json`)
   - Option B — site repo (`andrewrahman-com/design-system.md`)
   - Option C — a new shared repo/package under the Spatial-Media-Lab org

2. **What format?**
   - Markdown-only (human reference)
   - JSON tokens + CSS variables file (machine-consumable by site build + Patreon generator)
   - Storybook / design-tokens package (heaviest)

3. **Which surface is canonical for the drifted tokens?**
   - Site (most recent, most visible to users)
   - Plugin (longest-lived, but v1.0.0 already shipped — changes land in v1.1)
   - Pick a new value that works across both and update everything

4. **Retrofit scope**
   - Patreon `_generate.py`: low-cost, high-value — definitely retrofit
   - Plugin: v1.0.0 shipped, changes would land in v1.1 cycle — **flag only**
     unless the user wants a patch release
   - Site: becomes the "living reference" during Plan 03-01 execution

5. **Requirement ID**
   - Add `DESIGN-01` (or `WEB-04`) to REQUIREMENTS.md for style-guide existence
   - Phase 3 already owns WEB-01/02/03 — style guide is an addition, not a
     replacement

## Usage notes for the discussion phase

- The 12-tap rainbow can be treated as *settled* — no debate needed, it's
  already consistent everywhere.
- The genuine design question is the **supporting palette** (text greys,
  secondary accents) — which deserves conscious pick-one-and-commit.
- Plan 03-01 (website finalization, B-1…B-8 from 03-BACKLOG.md) will expose
  additional tokens as design choices get made (e.g. B-5 regal-purple Patreon
  CTA). Those should land in the style guide during Plan 03-02, not be
  designed ad-hoc again.
