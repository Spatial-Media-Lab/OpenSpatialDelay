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
| 6 | `screenshot_elevation_map.png` | ⏳ Awaiting review | — |
| 7 | `screenshot.png` | ⏳ Awaiting review | — |
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

## Next session / next item

The next item to review is **#6 — `screenshot_elevation_map.png`**.
Round 2 accepted it after the spiral flip and plugin-native label change;
any round-3 revision will come from direct user feedback on the current
PNG at `docs/assets/screenshot_elevation_map.png`.

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
