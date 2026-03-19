# OpenSpatialDelay User Manual Update Plan

## Context

The current manual (`docs/OpenSpatialDelay_Manual_v1.0.docx/pdf`) was drafted during v0.9 development. Several major features have been added or rewritten since the initial manual was created. This plan outlines the updates needed to bring the manual to v1.0 release quality, following modern audio plugin documentation standards.

## Content Updates Required

### Major Rewrites

| Section | What Changed | Priority |
|---------|-------------|----------|
| **Trajectory System** | Complete rewrite: 13 shapes (was 6), origin-point architecture, direction control (fwd/rev), crosshair origin markers, glow trails | P0 |
| **Factory Presets** | 60 presets across 9 categories (Ambient Spaces, Deep Dub, Experimental, Glitch & Stutter, Immersive FX, Rhythmic, Simple, Spatial Movement, Surround). Build-time installation. | P0 |
| **Signal Flow Diagram** | WSOLA-lite per-tap pitch, split pitch architecture (varispeed global + WSOLA per-tap). New signal flow diagram needed. | P0 |

### Section Updates

| Section | What Changed | Priority |
|---------|-------------|----------|
| **Per-Tap Controls** | Per-tap pitch shift, input channel (L+R/L/R), trajectory direction (fwd/rev) | P1 |
| **Getting Started** | New filter defaults (HP 50Hz, LP 5kHz, Res 0.71, filters OFF by default) | P1 |
| **Preset System** | PresetSaveOverlay (in-plugin modal), user preset save/load, category-based browsing | P1 |

### New Sections to Add

| Section | Description | Priority |
|---------|-------------|----------|
| **Stereo Input Routing** | Dual delay lines, per-tap L+R/L/R channel selection, INPUT button | P1 |
| **Wobble Modulation** | MOD section: wobbleEnabled, wobbleAmount, wobbleMorph parameters, waveform morphing | P1 |
| **Per-Tap Pitch** | Split architecture explanation (varispeed for cumulative, WSOLA for additive), additive mode | P1 |
| **Trajectory Reference** | Visual guide for all 13 shapes with behavior descriptions | P2 |

### No Changes Needed

| Section | Status |
|---------|--------|
| HRTF Profiles (6) | Unchanged |
| Output Formats (21) | Unchanged |
| ADM-OSC Receive/Send | Unchanged |
| Spatialization Algorithms (7) | Unchanged |
| Keyboard Shortcuts | Unchanged |

## Screenshot Requirements

Using the `screenshot_tool` CLI (`./build/screenshot_tool [path] [scale]`):

| Screenshot | Description | Preset/State |
|-----------|-------------|-------------|
| Default state | Full plugin with Binaural, 4 taps | Default (no preset) |
| Spatial map detail | Close-up of spatial map with trajectory active | Load Orbit Dance preset |
| Right panel | DELAY, TONE (with filter graph), MIX sections | Default |
| Bottom panel | Per-tap controls with pitch, trajectory, direction | Enable all columns |
| Header bar | Format dropdown, algorithm, OSC, SML badge | Show dropdown open |
| Preset save overlay | In-plugin modal for saving presets | Trigger save overlay |
| Each trajectory shape | 13 screenshots showing active trajectory path | One per shape |
| Stereo input | INPUT button and channel selection | Show L/R mode |
| Wobble modulation | MOD section with knobs active | Enable MOD |

## Design Standards Reference

Research these audio plugin manuals for modern documentation conventions:

### Industry Benchmarks
- **FabFilter** — Clean, concise manuals with annotated UI screenshots, parameter tables, signal flow diagrams. Considered gold standard.
- **Valhalla DSP** — Minimal documentation, but excellent parameter descriptions with musical context.
- **iZotope** — Comprehensive guides with workflow-oriented structure (Getting Started → Deep Dive → Reference).
- **Soundtoys** — Visual-first approach with large annotated screenshots and minimal text.
- **Output** — Modern, design-forward documentation with dark screenshots matching dark UIs.

### Style Guidelines for OSD Manual
1. **Dark screenshots** matching the plugin's dark UI (#0A0A14 background)
2. **Annotated UI images** with numbered callouts linking to parameter descriptions
3. **Signal flow diagrams** using the plugin's color system (cyan primary, purple secondary)
4. **Parameter tables** with: Name, Range, Default, Description columns
5. **Musical context** for DSP parameters (e.g., "At high feedback with pitch shift, creates shimmering ascending delays")
6. **Workflow sections** organized by use case (binaural production, surround mixing, live performance)

## Generation Toolchain

### Current Stack
- `docs/generate_manual.js` — Node.js script generating DOCX via `docx` npm package
- `docs/package.json` — Dependencies (`docx` v9.6.1)
- Output: `docs/OpenSpatialDelay_Manual_v1.0.docx` and `.pdf`

### Update Approach
1. Update `generate_manual.js` with new sections and screenshots
2. Run `npm install` in `docs/` to restore dependencies
3. Run `node generate_manual.js` to regenerate DOCX
4. Convert DOCX→PDF (macOS: `textutil` or LibreOffice CLI)
5. Review output and iterate

### Skill Support
- `/doc-coauthoring` — 3-phase documentation workflow (context → refinement → reader testing)
- `/docx` — DOCX generation with TOC, footnotes, embedded images, styled tables

## Timeline

| Phase | Task | Est. Duration |
|-------|------|---------------|
| 1 | Capture all screenshots with screenshot_tool | 1-2 hours |
| 2 | Update generate_manual.js with new content | 4-6 hours |
| 3 | Review and refine with doc-coauthoring skill | 2-3 hours |
| 4 | Final PDF generation and QA | 1 hour |

## Out of Scope
- Video tutorials (deferred to post-v1.0)
- Interactive web documentation (deferred)
- Localization (English only for v1.0)
- API/developer documentation (not user-facing)
