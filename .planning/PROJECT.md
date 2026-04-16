# OpenSpatialDelay

## What This Is

A spatial delay audio plugin where each echo lives in 3D space. VST3 + AU for macOS, VST3 for Windows. 7 spatialization algorithms, 5 HRTF binaural profiles, phase vocoder pitch shifting, trajectory engine, and Doppler simulation. Built with JUCE 8.0.3, dual-platform CI. Licensed GPL-3.0. The first product in the Spatial Media Library pipeline, developed under Spatial Media Lab.

## Brand Identity

Three distinct names appear in Phase 2+ artifacts and must never be conflated:

- **Spatial Media Lab (SML)** — the open-source organization that owns the source code at `github.com/Spatial-Media-Lab/OpenSpatialDelay` and the site `SpatialMediaLab.org`. Abbreviation "SML" refers to this entity only.
- **Spatial Media Library** — the pipeline brand for the set of spatial-audio tools funded via Patreon. OpenSpatialDelay is the first. Always written as three separate words. Do NOT abbreviate.
- **OpenSpatialDelay (OSD)** — the first plugin product in the Spatial Media Library pipeline, developed under Spatial Media Lab.
- **Andrew Rahman** — personal creator identity used on Patreon (`patreon.com/andrewrahman`) and `andrewrahman.com`.

Every externally-published Phase 2 artifact (Tally form copy, privacy policy, Patreon page, andrewrahman.com pages, seed posts) MUST disambiguate "Spatial Media Lab" from "Spatial Media Library" on first reference in any given document.

## Core Value

Each delay echo occupies a distinct spatial position, creating immersive 3D soundscapes that move through space around the listener.

## Current Milestone: v1.0 Release Plan

**Goal:** Launch OpenSpatialDelay publicly — website, distribution, marketing, legal, and Patreon funding infrastructure.

**Target features:**
- Personal website rebuild (explore static site with MagicUI/Impeccable vs. update existing builder)
- Patreon account for spatial audio tools pipeline funding
- Tally survey / mailing list (email capture leading to download)
- SpatialMediaLab.org About Us page update (add Andrew + board)
- SML blog post announcing OSD
- SML newsletter email announcing OSD
- LinkedIn announcement post
- Instagram announcement post
- Legal change: drop commercial license, GPL-3.0 only
- Video/audio demo content (scripts + storyboards)
- Documentation cleanup (ensure publish-ready)

## Requirements

### Validated

Shipped in v1.0.0 (768c248, 2026-04-12):

- Spatial delay engine with 12 independent echo objects in 3D space
- 7 spatialization algorithms (Ambisonics, ConstantPower, DBAP, KNN, MDAP, VBAP, VBIP) — see Source/PluginProcessor.cpp:1800
- 6 binauralization options: 5 measured HRTF profiles (MIT KEMAR, SADIE D2-KU100, CIPIC 003, HUTUBS PP2, Bernschuetz KU100) + 1 CPU-lite option "Simple (Low CPU)" — Woodworth ITD + geometric ILD approximation, not an HRTF
- Phase vocoder pitch shifting (WSOLA-Lite) with PDC compensation
- Trajectory engine — 13 shapes (Bounce, Circle, Cross, Figure-8, Heart, Helix, Infinity, Line, Orbit, Random, Spiral, Square, Triangle) — see Source/PluginProcessor.cpp:500
- Doppler velocity simulation with per-sample pitch interpolation
- Filter bank per echo object
- Partitioned convolution for HRTF rendering
- Preset system with undo/redo and host automation
- OSC send/receive for external spatial control
- VST3 + AU on macOS (arm64 + x86_64), VST3 on Windows (x64)
- 162+ Catch2 tests

### Active

Release Plan scope — see REQUIREMENTS.md for REQ-IDs.

### Out of Scope

- Plugin code changes (no new DSP features, no refactoring in this milestone)
- Mobile or web versions
- Paid plugin tiers (plugin is free; Patreon is the funding model)
- AAX format support

## Context

- **Pipeline:** OSD is the first in a planned series of spatial audio tools. The Patreon funds the entire pipeline, not just this plugin.
- **Audience:** Broad — music producers, sound designers, spatial audio researchers/artists, anyone working with audio.
- **Distribution model:** Free GPL-3.0 plugin. Email capture via Tally survey leads to download. Patreon supporters get direct feature request access and future exclusive plugins.
- **Phasing strategy:** Website + mailing list first (enables early social posts) → finish remaining items → mailing list blast + social drip.
- **Timeline:** Aspirational end-of-week (2026-04-18), realistic when-it's-ready.
- **Websites:** Both personal site and SpatialMediaLab.org are currently on website builders. Personal site may be rebuilt as a free static site (code-based).
- **Assets:** Plugin screenshots exist (screenshot_tool + user guide). Video and audio demos need creation.

## Constraints

- **Budget**: Personal website hosting must be free (GitHub Pages, Netlify, or similar)
- **Platform**: SML website is a website builder — changes are content/copy, not code
- **Legal**: JUCE GPL-3.0 tier is compatible with dropping the commercial license
- **Brand**: OSD should be positioned as the first tool in a spatial audio suite, not a standalone product

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Drop commercial license, GPL-3.0 only | Plugin is free; revenue comes from Patreon supporters, not per-seat sales | Validated in Phase 1 (2026-04-15) — repo/docs clean, release asset compliant |
| Patreon as funding model | Funds entire spatial audio tools pipeline; supporters get feature requests + future exclusive plugins | -- Pending |
| Tally for email capture | Free tier, lightweight, no code needed | -- Pending |
| Website + mailing list first | Enables social posts immediately; rest can ship later | -- Pending |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? -> Move to Out of Scope with reason
2. Requirements validated? -> Move to Validated with phase reference
3. New requirements emerged? -> Add to Active
4. Decisions to log? -> Add to Key Decisions
5. "What This Is" still accurate? -> Update if drifted

**After each milestone** (via `/gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check -- still the right priority?
3. Audit Out of Scope -- reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-04-14 after milestone v1.0 initialization*
