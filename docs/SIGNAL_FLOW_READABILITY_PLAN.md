# Signal-Flow Diagram Readability Plan

**File**: `docs/assets/signal-flow.png`
**Generator**: `docs/generate_signal_flow.py`
**Canvas**: 2400×1400 (fixed)

## Observed readability issues

1. **Overlapping block text in the Spatialization Engine panel.** `HRTF Convolution`
   and `Simple Stereo` render on top of each other in the same cell, and the
   `PartitionedConvolver + ITD / 6 HRTF profiles` sublabel reads as corrupted
   text because of the collision. This is the worst single readability issue.
2. **Very small body copy vs. canvas size.** The canvas is 2400×1400 but most
   sublabels are 12–14 px (DM Sans Regular / JetBrains Mono 12–13). At the size
   the PNG is actually displayed (github README, manual PDF), the sublabels
   drop below the legibility threshold.
3. **Low-contrast helper colours.** `DIM = (100, 105, 120)` is used for every
   secondary label (sublabels, bottom footer, side notes). It is only ~2.3:1
   against the `#0A0A14` background — below WCAG AA for normal text.
4. **Long free-floating annotation lines.** The "back to delay input",
   "mono sources", "Output: 1-49 channels | Stereo | Binaural | Ambisonics"
   labels sit without a background panel behind them, so they overlap grid
   lines and other arrow strokes.
5. **Legend colour swatches too short.** The three legend strokes (main /
   dry / feedback) are short line segments with no label weighting — easy to
   miss as a legend at first glance.
6. **Inconsistent typographic hierarchy in the Per-Tap / Spatialization /
   Feedback Path section headers.** They use DM Sans Bold 22 but no visual
   separator or rule underneath. The eye doesn't register them as section
   titles vs. block titles.
7. **Bottom info panels ("Phase Vocoder Details", "Tape Wobble Emulation")**
   are four-line dense monospace blocks with no framing. They compete with the
   diagram boxes for attention and make the bottom of the canvas feel busy.

## Proposed fix plan (not executed yet)

Step-ordered so each step produces a visibly better diagram on its own:

### Step 1 — Fix the HRTF / Simple Stereo overlap
Split the Spatialization Engine panel into a **2×4** grid (currently 2×3 with
the fourth row shared). Layout becomes:

```
VBAP      DBAP
VBIP      Ambisonics
MDAP      HRTF Convolution
KNN       Simple Stereo
```

Each block sized to a consistent height so no sublabel collides. The
`PartitionedConvolver + ITD / 6 HRTF profiles` sublabel moves under
`HRTF Convolution` only; `Simple Stereo` gets its own short sublabel
(e.g. `pan law, no filtering`).

### Step 2 — Typography scale bump (2×)
Because this PNG is displayed at half canvas size in the manual and README:
- Block titles: 22 → 28
- Sublabels: 14 → 18
- JetBrains Mono body: 12 → 14
- Footer and free-floating labels: 13 → 16

Canvas grows to 2800×1600 to keep whitespace proportional (or keep canvas and
accept tighter margins — decision during execution).

### Step 3 — Contrast bump on secondary text
Replace `DIM = (100, 105, 120)` with `DIM = (140, 148, 164)` which is ~3.4:1
against the background and clears WCAG AA for large text. Also bump
sublabel colour from `DIM` to `WHITE @ 70% alpha` for block sublabels so they
don't disappear on retina-scaled renders.

### Step 4 — Panel annotations behind a thin rule
Wrap "back to delay input", "mono sources", and "Output: 1-49 channels | …"
in a 1-px stroked rounded rect filled with `BG` so they punch through grid
lines. No colour change needed.

### Step 5 — Legend visual weight
Grow legend swatch length from ~40 px to 80 px, add a thin background panel
behind the legend block, and bump labels to DM Sans Medium 18.

### Step 6 — Section headers
Add a 1-px `CYAN_DIM` horizontal rule 8 px below each of `Per-Tap Processing`,
`Spatialization Engine`, `Feedback Path` titles. Same for `Phase Vocoder
Details` and `Tape Wobble Emulation` bottom-left info panels (or give those
panels a subtle border).

### Step 7 — Bottom info panels framed
Give `Phase Vocoder Details` and `Tape Wobble Emulation` proper rounded-rect
frames so they read as reference callouts rather than loose body text.

## Decision points before execution

- **Canvas size**: keep at 2400×1400 and tighten margins, or grow to
  2800×1600? Affects README display size. Recommend growing.
- **Font bump**: straight 2× or a targeted bump of just the body copy?
  Recommend targeted — block titles already work.
- **Colour palette**: preserve the cyan / violet / amber mapping
  (main / feedback / dry) or introduce a fourth hue for the submenu blocks?
  Recommend preserve.

None of Steps 1–7 should be started until the palette and canvas decisions are
locked. Step 1 alone closes the worst reported issue; Steps 2–3 are the
highest-value follow-ups.
