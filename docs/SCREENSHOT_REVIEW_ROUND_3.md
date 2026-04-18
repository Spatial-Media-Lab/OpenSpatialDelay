# Screenshot Review — Round 3 tracker (Issue #168)

Round 3 is an iterative, item-by-item revision pass on top of the round-2
commits (`904fa8a`, `b6a4db9`) on `AndrewRahman/screenshots-r2`. Unlike
rounds 1 and 2 — which were batched reviews delivered as a single issue
comment — round 3 is conducted live in the session: the user reviews one
annotated screenshot at a time, each item lands as its own atomic commit,
and this file tracks which items have been addressed and which remain.

Prior rounds:
- Round 1 review: issue #168 comment 2026-04-17T12:48Z
- Round 2 review: issue #168 comment 2026-04-17T13:57Z
  (see `docs/SCREENSHOT_REVIEW_ROUND_2.md` for the full round-2 handoff)
- Round 2 landing summary: issue #168 comment 2026-04-17T16:06Z

## Item status

The eight screenshots that round 2 flagged for revision carry forward into
round 3. User iterates per-item; this table tracks state.

| # | File | Status | Round-3 notes |
|---|------|--------|---------------|
| 2 | `screenshot_annotated.png` | ✅ Done (`063c9f6`) | see "Item 2 — Annotated screenshot" below |
| 6 | `screenshot_elevation_map.png` | ✅ Done (no change) | Round-3 review: accepted as-is |
| 7 | `screenshot.png` | ✅ Done (this commit) | see "Item 7 — Hero screenshot" below |
| 10 | `screenshot_output_menu.png` | ⏳ Awaiting review | — |
| 11 | `screenshot_preset_menu.png` | ⏳ Awaiting review | — |
| 12 | `screenshot_right_panel.png` | ⏳ Awaiting review | — |
| 16 | `screenshot_tone_section.png` | ⏳ Awaiting review | — |
| 17 | `screenshot_undo_active.png` | ⏳ Awaiting review | — |

Investigation-only: `screenshot_shimmer.png` (14) — confirmed still in use
on the personal site; no change required.

## Item 2 — Annotated screenshot

Reviewed and revised in this session. Source of truth for the revised
state is `tools/screenshot_tool.cpp` (new `annotated-source` capture mode)
and `docs/assets/annotate_screenshot.py` (callout coordinates, legend,
supersampled drawing).

**What changed versus the round-2 drop:**

- **New `annotated-source` capture mode** in `screenshot_tool.cpp`.
  Produces a dedicated source image for the annotated overlay by combining
  the showcase state with the Global Drawer open and a populated undo
  history — drawer visible on the left, undo arrow highlighted in the
  header. `annotate_screenshot.py` now reads this as its `SRC`, so the
  main `screenshot.png` can keep the drawer closed for the other PIL
  crops.
- **Supersampled overlay** (3× canvas resolution, LANCZOS downscale) in
  `annotate_screenshot.py`. Circles and leader lines no longer look
  pixelated — Pillow's primitives have no native AA.
- **Legend labels rewritten** per the user's wording:
  - HEADER: … / Algorithm / HRTF Profile
  - MAP: Global Controls / Spatial Map
  - RIGHT PANEL: Delay controls / Modulation controls / Filter controls
    / Output controls / OSC Send/Receive
  - PER-TAP: Tap Selector / Per-Tap Controls / Trajectory Controls
- **Callout 8** horizontal leader to the `0.25` distance-ring label in
  the spatial map background — target (662, 572) source.
- **Callouts 9–13** now straight horizontal leaders from the right margin
  at the exact target Y, pointing to each section's *centre* rather than
  its header bar:
  - 9 → `1/4` value under TIME
  - 10 → midpoint of AMOUNT/MORPH knobs
  - 11 → centre of filter graph
  - 12 → midpoint of DRY/WET + OUTPUT
  - 13 → point between receive port and send IP/port
- **Callout 14** horizontal leader directly at the "1" digit on Tap 1.
- **Callouts 15, 16** sit in the bottom-left / bottom-right corners of
  the canvas (source y=1277, mid-way between the image bottom and the
  legend bottom), with diagonal leaders that terminate between the
  Elevation/Distance knobs (15) and between the Trajectory column and
  the Speed knob (16).
- **Draw order** reversed — legend is drawn first, leader lines + circles
  rendered on top. Ensures the 15/16 diagonals remain visible if they
  cross any legend geometry.

## Item 6 — Elevation map

Reviewed in this session against the current PNG at
`docs/assets/screenshot_elevation_map.png`. Accepted as-is — no changes
required. The round-2 spiral flip and plugin-native label change are
considered final for round 3. Item 6 is closed and needs no further
work.

## Item 7 — Hero screenshot (`screenshot.png`)

Reviewed and revised iteratively in this session. The round-2 output
was a plain `--mode full --showcase` capture; round 3 upgrades it to a
new dedicated `hero` mode that renders the full "features-on" demo
plus narrative elements the round-2 image lacked (drawer open, undo
lit, tap activity glow, a named custom preset, and a visible Infinity
trajectory). Source of truth is `tools/screenshot_tool.cpp` (new
`hero` mode + reworked `applyShowcaseState`), `Source/PluginEditor.*`
(three new screenshot hooks), and `scripts/regenerate_screenshots.sh`
(line 49 now calls `--mode hero`).

**What changed versus the round-2 `--mode full --showcase` drop:**

- **New `hero` capture mode** in `screenshot_tool.cpp`. Combines
  `applyShowcaseState` + `configureGlobalDrawer(true, …)` +
  `populateUndoHistory` + post-sync overrides for preset name, per-tap
  activity and the selected object's trajectory state. `--showcase`
  is implicit so the script doesn't need both flags.
- **`applyShowcaseState` reworked** to match the round-3 hero spec:
  - Tap 1 now carries an **Infinity** trajectory (shape 7, not Orbit),
    with the lead routed from the **L** channel (inputChannel=1,
    not the default L+R), **+7 st** pitch shift and **75 %** Doppler.
  - Tap 1 is centred on the listener axis: `az=0°, el=+20°,
    dist=0.25` — so the lemniscate sweeps symmetrically.
  - Taps **3, 6, 9 are disabled**; taps **10, 11, 12 are enabled** in
    their place so the scene still carries 9 active echoes.
  - Non-Tap-1 taps are hand-placed *outside* the lemniscate bounding
    region (mapX ∈ [-0.75, 0.75], mapY ∈ [-0.015, 0.515]) at varied
    distances (0.42–0.95) and elevations (−55° to +72°) so the spread
    reads 3D rather than a ring.
- **Three new screenshot hooks in the editor** (`PluginEditor.h/.cpp`):
  - `SpatialMapComponent::setDrawFullTrajectoryForScreenshot(bool)` —
    when true, the selected tap's trajectory trail keeps its
    proximity-focused bright spot near the animated dot but lifts the
    baseline from 0.05 → 0.18 so the rest of the path is lightly
    visible in a still frame.
  - `OpenSpatialDelayEditor::setPresetNameForScreenshot(String)` —
    manual override for the preset-name button so the snapshot shows
    "Infinity Halo" instead of the underlying preset index's name.
  - `OpenSpatialDelayEditor::applyShowcaseHeaderForScreenshot()`
    extended to re-pull processor state into the OSC Receive / Send
    toggle buttons — they're initialised once in the editor
    constructor, so without this resync the hero image kept the
    wrong lit/dim pattern after `setOscReceiveEnabled(false)` /
    `setOscSendEnabled(true)`.
- **Per-tap activity glow** seeded on taps 2, 5, 8, 11
  (`setObjectActivityLevel` at 0.65–0.85), so the hero image reads as
  a plugin in motion rather than a dead layout.
- **TrajectoryState seeded manually** for Tap 1 in the tool — the
  live view gets this from `trajectory.tick()` inside `processBlock`,
  which never runs in the offline tool.
- **Custom preset label "Infinity Halo"** displayed in the header.
- **`scripts/regenerate_screenshots.sh`** line 49 switched from
  `--mode full --showcase` to `--mode hero`. Derived PIL crops
  (`screenshot_header.png`, `screenshot_right_panel.png`,
  `screenshot_bottom_panel.png`) and the `screenshot_full.png` copy
  will now carry hero-state content when the script is next run.

**Downstream cascade to watch in the next session:**

Because `screenshot.png` is the PIL-crop source for three other PNGs
(header / right_panel / bottom_panel) and is also copied to
`screenshot_full.png`, those four files are now out of sync with the
new hero state on disk. Regenerate them from the new `screenshot.png`
before reviewing the remaining round-3 items — in particular
**#12 `screenshot_right_panel.png`** depends on this source and
cannot be reviewed against the old round-2 crop.

## Next session / next item

Item 7 is closed. Remaining open items: **10, 11, 12, 16, 17**. User
has been choosing the next item to review, so the next session should
prompt with the outstanding list and wait for the pick. Note the
downstream cascade above before touching item 12.

## Files the next session needs

- Branch `AndrewRahman/screenshots-r2` (checked out in the `tallinn`
  worktree — this session has been editing through a mirror at
  `.context/edits/` in the `doha` worktree because the doha conductor
  workspace is pinned there).
- `docs/SCREENSHOT_REVIEW_ROUND_2.md` — upstream context.
- `docs/SCREENSHOT_REVIEW_ROUND_3.md` — this tracker.
- `docs/assets/screenshot_*.png` — current PNG state.
- `.context/attachments/` — round-2 reference screenshots (still relevant
  for any preset-menu / output-menu re-examination).
