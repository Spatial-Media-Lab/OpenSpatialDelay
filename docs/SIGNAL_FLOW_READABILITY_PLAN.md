# Signal-Flow Diagram Readability Plan

**File**: `docs/assets/signal-flow.png`
**Generator**: `docs/generate_signal_flow.py`
**Current canvas**: 2000×1180 (down from original 2400×1400)

## Status

**In progress — not shipped as finished.** Enough rework has landed to
close the worst structural/readability issues, but the diagram still has
visible empty zones inside the canvas. Pick this up in the next session
and keep tightening until it feels "done" end-to-end.

## What has been done (see commit history on `AndrewRahman/issue-168-backlog`)

Structural fixes (11-item list from original review):

1. ✅ HRTF Convolution / Simple Stereo overlap fixed — Spatialization Engine
   is now a clean 2×4 grid, no floating annotations.
2. ✅ "mono sources" label placed in a BG-filled pill so it punches
   through panel borders.
3. ✅ Orphan "Output: 1-49 channels | Stereo | Binaural | Ambisonics"
   strip removed; format info absorbed into Spat panel subtitle.
4. ✅ Per-Tap and Feedback intra-group arrows bumped to width=3.
5. ✅ Dry path rerouted: short arrow from Stereo Input split into a
   wide-flat Latency Compensation bar, then straight down the narrow
   right-gutter into Crossfade from the right.
6. ✅ Feedback return rerouted through the inter-panel corridor (between
   Spat and Feedback panels) with a "back to delay input" pill above
   Spat panel's top.
7. ✅ Removed the misleading direct arrow `Dual Delay Lines →
   Spatialization Engine` — spat is fed only via Per-Tap output.
8. ✅ Orphan "Phase Vocoder Details" and "Tape Wobble Emulation"
   callouts dropped.
9. ✅ Legend gained a 4th entry explaining group-header colour semantics.
10. ✅ "x12 taps per delay line" replaced with "12 taps × 2 delay lines
    (L/R)".
11. ✅ Legend relocated to bottom-left.

Layout tightening (after user pushed back on initial canvas reduction):

- Canvas 2400×1400 → 2000×1180 (~30% area reduction).
- All x-coordinates shifted left by 80 (tap_panel_x=40, input_x=970,
  spat_panel_x=800, fb_panel_x=1480, corridor_x=1460, lg_x=60).
- Latency Compensation redesigned as a **wide horizontal bar**
  (680×32 at x=1280–1960, y=split_y centred) rather than a 320×50
  block floating in empty space. The dry-path horizontal gap is now
  filled by the bar itself, and the bar's right edge meets the dry
  descent lane directly (no +40 horizontal offset).

## What still needs attention

The user called the current state unfinished. Voids that remain:

1. **Top-right zone below Latency Compensation bar, above Feedback
   panel header** — roughly y=232–454, x=1180–1960 = ~820×220 empty
   region. Candidates: move Legend up into this space, or shrink the
   vertical chain and pull panels up.
2. **Bottom-right zone below Feedback panel** — y=854–1180, x=1320–2000.
   Feedback panel has ~112 px of internal bottom padding because it
   only holds 3 blocks (Soft Clipper / HP-LP / Feedback Gain) while
   Per-Tap has 5 and Spat has 8. Options:
   - Let Feedback panel be shorter than Per-Tap/Spat (break equal-height
     convention, honest visual weight).
   - Drop panel height globally (h=400 → h=380 or lower) if Per-Tap
     block height can tighten.
3. **Left-of-chain zone** — y=130–430, x=40–870 (left of the vertical
   Stereo Input → Input Gain → Dual Delay Lines chain, above Per-Tap
   panel). Currently dead space above the Per-Tap panel and to the
   left of the main chain.
4. **Vertical chain gaps** — `split_y→ig_y = 55`, `ig_bottom→dd_y = 40`,
   `dd_bottom→branch_y = 45`. Each could trim ~10–15 px.
5. **Legend could be compacted** — currently 540×170. A
   horizontal-row legend would be tighter.

## Decision points for next session

Before touching more code, the user should pick a direction:

1. **Legend placement**: leave bottom-left (anchors the composition) or
   move to top-right below LC bar (fills the biggest remaining void but
   opens a bottom-left void)?
2. **Panel height policy**: all-equal-h=400 (current) or
   content-sized-per-panel (Feedback shorter)?
3. **Aspect target**: is 2000×1180 (~17:10) acceptable, or aim for a
   tighter 16:9 / 3:2 target?

None of the remaining work is required for the diagram to be
functionally correct. This is pure compositional tightening.

## Files touched

- `docs/generate_signal_flow.py` — full rewrite of layout routing and
  Latency Comp representation.
- `docs/assets/signal-flow.png` — regenerated.
- `docs/OpenSpatialDelay_Manual_v1.0.docx` / `.pdf` — regenerated to
  pick up the new diagram in Section 2 "Signal Flow".

## Regenerate commands

```
python3 docs/generate_signal_flow.py
NODE_PATH=/opt/homebrew/lib/node_modules node docs/generate_manual.js
soffice --headless --convert-to pdf docs/OpenSpatialDelay_Manual_v1.0.docx \
  --outdir docs/
```
