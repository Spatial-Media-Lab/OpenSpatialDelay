# Signal-Flow Diagram Readability Plan

**File**: `docs/assets/signal-flow.png`
**Generator**: `docs/generate_signal_flow.py`
**Current canvas**: 2400 × 900 (8:3 aspect) — pass 4 "Orbital Horizon"

## Status

**Pass 4 shipped — awaiting user sign-off.** The diagram was redrawn
from scratch on branch `AndrewRahman/flow-diagram-v4` after passes 1-3
were judged "not greatly improved." The rewrite fixes a latent bug and
re-stages the composition around the brand's stated design philosophy.

The DOCX and PDF have **not** been regenerated — per the "no manual
regen until images final" rule, the new PNG must be reviewed first.

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
- INPUT and OUTPUT at 30 pt; panel titles at 26 pt. Chosen so the
  hierarchy survives the 5:1 scale-down to the manual's 480 × 180 pt
  render.

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

- `docs/generate_signal_flow.py` — complete rewrite. New geometry
  budget, cubic-Bezier path helpers, orbital-ring / angle-tick
  drawing, capability-level label rewording, 8:3 canvas constants.
- `docs/assets/signal-flow.png` — regenerated.
- `docs/SIGNAL_FLOW_READABILITY_PLAN.md` — this document.

**NOT regenerated** (awaiting user sign-off on the PNG):

- `docs/OpenSpatialDelay_Manual_v1.0.docx`
- `docs/OpenSpatialDelay_Manual_v1.0.pdf`

## Regenerate commands (after PNG is signed off)

```
python3 docs/generate_signal_flow.py
NODE_PATH=/opt/homebrew/lib/node_modules node docs/generate_manual.js
soffice --headless --convert-to pdf docs/OpenSpatialDelay_Manual_v1.0.docx \
  --outdir docs/
```

## Commit trail

Pass 4 work lives on `AndrewRahman/flow-diagram-v4`, rebased from
`AndrewRahman/signal-flow-polish` (which was 5 commits ahead of main
when pass 4 began).
