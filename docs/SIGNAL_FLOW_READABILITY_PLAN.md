# Signal-Flow Diagram Readability Plan

**File**: `docs/assets/signal-flow.png`
**Generator**: `docs/generate_signal_flow.py`
**Current output**: 4800 × 1800 (logical 2400 × 900 × SCALE=2)
**Aspect**: 8:3, locked to the DOCX manual render box
**Status**: **Pass 4 signed off by user** — ready for manual regen.

## Status

Pass 4 ("Orbital Horizon") was signed off by the user. The diagram
was redrawn from scratch on branch `AndrewRahman/flow-diagram-v4`
after passes 1-3 were judged "not greatly improved." The rewrite
fixes a latent aspect-ratio bug and re-stages the composition around
the brand's stated design philosophy. A follow-up commit doubled the
render DPI without changing geometry.

The DOCX and PDF still need to be regenerated to pick up the new
PNG — see "Next session" at the bottom.

## Root cause found during pass-4 research

Two research agents produced structured reports. The critical
discovery was in the rendered manual itself:

`docs/generate_manual.js` transforms the image to
`{ width: 480, height: 180 }` when embedding it in the DOCX. That's
an 8:3 (≈ 2.67:1) aspect ratio. Every previous pass drew the PNG
at ~16:9 (2000 × 1180, 1920 × 1080, 2400 × 1400), so the diagram was
being **horizontally compressed** by roughly 50 % inside the A4
manual at every rendering. This explains the persistent sense that
the layouts never quite "worked" — the geometry the user saw in the
PDF was not the geometry the generator produced.

Independent of the aspect bug, the "three panels side-by-side"
paradigm was never reconsidered across passes 1-3 even though the
Orbital Cartography design philosophy calls for **radial / orbital**
composition, not static panels.

## What pass 4 did

**Geometry:**
- Canvas 2400 × 900 (exactly 8:3) — matches the manual's DOCX
  transformation, eliminating the horizontal compression.
- Horizontal left-to-right signal flow. This is the first pass to
  try the horizontal-entry-chain paradigm flagged as untried in the
  previous plan.
- The 12-tap fan radiates from the DELAY BUFFER output as the
  compositional centre — honours "every layout radiates from a
  centre" in the philosophy doc.

**Path routing:**
- Dry path is a single cubic Bezier arc sweeping above the main row
  (AMBER) — a "computed trajectory", not a right-angle route.
- Feedback is a cubic Bezier arc sweeping below the main row
  (VIOLET), tapping from the DELAY BUFFER bottom and re-entering the
  summer ⊕ from below. Matches the code truth: feedback is mono,
  pre-DSP, pre-spatialisation.
- Main signal path is short straight cyan arrows between blocks
  with a feedback summer ⊕ glyph where feedback rejoins.

**Orbital detailing:**
- Concentric rings at the fan origin (110, 200, 260, 320, 380, 440,
  500 px radii) form the "astronomical chart" field.
- Radial tick marks at ±30° on the outermost ring, with clinical
  mono angle labels (-30°, +30°).
- Corner registration crosshairs — chart-style alignment markers.

**Labels:**
- Capability-level only. "Pitch Shift" not "Phase Vocoder";
  "Woodworth" with the UI sublabel "simple (low CPU)" instead of
  the dataset name; no FFT size, no STFT, no Laroche-Dolson. Matches
  the marketing-copy-depth rule.
- Dry path pill annotates "2048-sample latency compensation"
  (user-visible fact).
- Feedback arc pill annotates "filter · saturate · gain".
- "limiter · wet path only" pill below MIX — honest about the
  dry-bypasses-limiter rule.

**Typography:**
- DM Sans (bold, semibold, medium, regular) for structural / block
  labels — the "generous scale" tier of the dual-extreme rule.
- JetBrains Mono for annotations, coordinate labels, tap indices —
  the "clinical small" tier.
- INPUT and OUTPUT at 30 pt (logical); panel titles at 26 pt. Chosen
  so the hierarchy survives the 5:1 scale-down to the manual's
  480 × 180 pt render.

**Resolution:**
- `SCALE` constant at the top of the generator multiplies every
  pixel-based value (canvas, geometry, fonts, line widths, paddings,
  radii). Text is re-rasterised at the scaled font size, so it stays
  crisp at any SCALE.
- Shipped at `SCALE = 2` → 4800 × 1800 PNG (~262 KB). Drop to
  `SCALE = 1` for the 2400 × 900 baseline or bump to `SCALE = 3` for
  7200 × 2700 without touching any other constant.

## Structural truths preserved

The diagram is verified against the actual DSP chain in
`Source/PluginProcessor.cpp`:

- Dry compensation is unconditional (2048 samples, pre-input-selector).
- Feedback is mono, reads from the delay buffer itself, rejoins
  before the buffer write.
- 12 taps read from one shared delay buffer at offsets
  `(N + 1) × baseDelay` — shown as the radial fan.
- Each tap goes through its own PV pitch shift → Doppler → air
  absorption → HP/LP filter, then to spatial render.
- Spatial render has 5 mutually-exclusive output paths. All six
  HRTF profiles (including Woodworth as Simple / Low CPU) are
  visible.
- Limiter is wet-only. Dry bypasses it.
- MIX is equal-power crossfade.

No DSP truth has been elided or misrepresented.

## Files touched in pass 4

- `docs/generate_signal_flow.py` — complete rewrite plus a SCALE
  multiplier pass. New geometry budget, cubic-Bezier path helpers,
  orbital-ring / angle-tick drawing, capability-level label
  rewording, 8:3 canvas constants, single-knob DPI.
- `docs/assets/signal-flow.png` — regenerated at 4800 × 1800.
- `docs/SIGNAL_FLOW_READABILITY_PLAN.md` — this document.

**NOT yet regenerated** (needs one final session):

- `docs/OpenSpatialDelay_Manual_v1.0.docx`
- `docs/OpenSpatialDelay_Manual_v1.0.pdf`

## Regenerate commands

```
python3 docs/generate_signal_flow.py
NODE_PATH=/opt/homebrew/lib/node_modules node docs/generate_manual.js
soffice --headless --convert-to pdf docs/OpenSpatialDelay_Manual_v1.0.docx \
  --outdir docs/
```

## Commit trail

Pass 4 work on `AndrewRahman/flow-diagram-v4`:

- `5efdb53` — pass 4 complete redraw at 8:3.
- `e84f486` — render at 2× DPI (4800 × 1800).

The branch was rebased from `AndrewRahman/signal-flow-polish`
(which was 5 commits ahead of main when pass 4 began).

## Next session

The PNG is final. The one remaining task is to regenerate the DOCX
and PDF to pick up the new diagram and verify it reads cleanly
inside the A4 page at the 480 × 180 pt render size. See the prompt
stub in `.planning/NEXT_SESSION.md` (if present) or the session-end
summary.
