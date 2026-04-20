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
| 10 | `screenshot_output_menu.png` | ⚠️ HITL (live capture) | see "Item 10 — HITL" below |
| 11 | `screenshot_preset_menu.png` | ⏳ Awaiting review | — |
| 12 | `screenshot_right_panel.png` | ✅ Done (this commit) | see "Item 12 — Right panel" below |
| 16 | `screenshot_tone_section.png` | ✅ Done (this commit) | see "Item 16 — TONE section crop" below |
| 17 | `screenshot_undo_active.png` | ✅ Done (this commit) | see "Item 17 — Undo/redo documented" below |
| 18 | `hero-plugin.{webm,mp4}` (new) | ✅ Encoded, site wiring pending | see "Item 18 — Hero video (Merry-Go-Round)" below |

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

## Item 10 — HITL (agent failure, live capture used instead)

**Outcome:** `docs/assets/screenshot_output_menu.png` is a **live
capture from the running plugin**, not a generated mock. The agent
could not produce a mock that matched the real JUCE ComboBox popup
within an acceptable iteration budget, so the user provided the
real screenshot (`.context/attachments/Screenshot 2026-04-17 at
15.50.02-v1.png`) and it was copied in verbatim.

**Why the agent failed:**

Three successive mock iterations could not match the user's reference:

1. **Iteration 1** (`8503747`): carried the round-2 spec forward —
   current item duplicated at top + separator + full list. Matched the
   round-2 written spec but not the real popup. Also had an
   argument-order bug (`isTicked` / `hasSubMenu` swapped in the call
   to `OSDLookAndFeel::drawPopupMenuItem`) so the tick glyph never
   rendered.
2. **Iteration 2** (`840db71`): fixed the swap, dropped the duplicate
   and separator, added a subtle 8%-white wash behind the ticked row.
   Structure was closer to the reference but the user confirmed the
   font rendering and selected-row treatment were still visibly off.
3. **Iteration 3 (abandoned):** no further changes landed — the user
   declared the mock-based approach a failure and requested HITL.

Root causes of the agent's misreads:

- Misinterpreted the highlighted top row in the reference as a
  "duplicate + separator" shape (carry-over from the round-2 written
  spec). The real popup is just the natural flat list; the item at
  position 1 happens to be Binaural because it is index 0 in
  `outputFormatRegistry` and is currently selected for the Quad
  Ping-Pong preset.
- Could not reproduce the real popup's font metrics and highlight
  fidelity through `PopupMenuSnapshot` + `OSDLookAndFeel::
  drawPopupMenuItem` without pumping a full message loop through
  JUCE's real `ComboBox::showPopup()` / `PopupMenu::showMenuAsync()`
  path — which the screenshot tool is not structured to do.

**HITL process for this screenshot going forward:**

1. The generated file is **not** produced by `regenerate_screenshots.sh`.
   Line 71 of that script is commented out with a pointer to this
   section; the `--mode output-dropdown` capture mode in
   `tools/screenshot_tool.cpp` and `buildOutputDropdownMock` are
   retained for possible future reference but are unused by the docs
   pipeline.
2. If the plugin's output-format list, layout, or LookAndFeel changes
   such that `docs/assets/screenshot_output_menu.png` goes stale, a
   human must re-capture it from the live plugin:
   - Open the plugin in a DAW at the native 820×580 editor size
     (2× for the retina capture — final PNG should be 1640×1160).
   - Click the OUTPUT Format combo box to open the popup.
   - Capture the editor window (screenshot or `screencapture -iw`).
   - Save to `docs/assets/screenshot_output_menu.png`.
3. Commit the new PNG directly. No code change required.

**Retry criteria (if an agent attempts the mock again):**

- Drive the real `ComboBox::showPopup()` from the tool (requires a
  headless message pump pumped for ~3 frames so JUCE can size and
  paint the popup).
- Snapshot the popup component *in-place* rather than rebuilding it
  from the `PopupMenu::Item` API.
- Compare against `.context/attachments/Screenshot 2026-04-17 at
  15.50.02-v1.png` at ≥200 % zoom pixel-diff before claiming a match.

## Item 12 — Right panel

**Outcome:** passed as-is on the first review pass this session — no
code changes required. The item-7 hero state already carries every
toggle the round-2 spec demanded; the round-3 session simply re-ran
`scripts/regenerate_screenshots.sh` so the PIL cascade picked up the
new `screenshot.png` and produced a current `screenshot_right_panel.png`.

**What the crop shows (round-2 spec lines 159–173):**

- DELAY — Sync **on** (yellow), Triplet **on** (lit 3-notes toggle);
  TIME 1/4, FEEDBACK 55%.
- MOD — toggle **on**; Amount 55%, Morph 40% (both non-zero).
- TONE / FLT — toggle **on**; filter graph shows a clear asymmetric
  Q curve: HP Res 0.10 (flat slope) paired with LP Res 8.00 (sharp
  resonant peak around 4 kHz).
- MIX / AIR — toggle **on**; DRY/WET 65%, OUTPUT 0.0 dB.

Crop bounds from the Python block in `regenerate_screenshots.sh`
(x 1112–1640, y 104–1018) were already correct from round 2; no
change to the crop maths.

**Ancillary files synced in this commit (regen-only, no semantic
change):** `screenshot_full.png` (copy of `screenshot.png`, was out
of sync after item 7), `screenshot_header.png` and
`screenshot_bottom_panel.png` (sibling PIL crops from the same
cascade), and `screenshot_annotated_source.png` +
`screenshot_annotated.png` (the hero-state-plus-drawer source for the
annotated overlay and its PIL output — re-emitted from the same regen
run so they agree with the current code).

## Item 16 — TONE section crop

**Outcome:** `getToneSectionBoundsForScreenshot()` in
`Source/PluginEditor.cpp` had `padBottom` reduced from 6 to 0. At 6 px
the top edge of the MIX / AIR pill below TONE was bleeding into the
bottom-right of the crop; 0 px makes the bottom cut flush at the
readout line ("HP 120  Res 0.10   LP 4.0k  Res 8.00"). No other layout
constants changed — the top padding, horizontal padding, and header /
graph / readout heights all keep the round-2 values.

The round-2 content requirements (FLT **on**, min-Q one side / max-Q
the other) were already satisfied via the hero cascade — this item is
purely a crop-bounds tighten.

**Files in this commit:**

- `Source/PluginEditor.cpp` — `padBottom = 0` + comment annotation
  explaining the round-3 reason.
- `docs/assets/screenshot_tone_section.png` — regenerated from the
  rebuilt `screenshot_tool` via
  `./build/screenshot_tool docs/assets/screenshot_tone_section.png 2.0
  --preset "Quad Ping-Pong" --mode tone-section --showcase`.

## Item 17 — Undo/redo documented

**Outcome:** the round-2 code fix (remove the trailing
`performInternalUndo()` call from `populateUndoHistory()` so only the
undo arrow is active) already landed in commit `904fa8a` — the
`screenshot_undo_active.png` asset was correct on disk for this
round. The round-3 work that closes the item is therefore *not*
another edit to the capture tool; it is documentation + a crop
refinement to the asset that the user-facing docs actually consume.

**Orphan-asset note:** `screenshot_undo_active.png` itself is
produced by `scripts/regenerate_screenshots.sh:72` but is not
currently embedded by any user-facing file — not
`docs/generate_manual.js`, not `docs/wiki/`, not the README, not the
andrewrahman.com site. It was added in round 1 as part of a batch of
"tool-capability" capture modes. Kept in place (still produced by the
regen script) so a future manual-update pass can claim it; no
deletion in this commit.

**What user-facing docs got instead:**

`docs/generate_manual.js` already embeds `screenshot_header.png` in
the "Controls Reference → Header Bar" section of the .docx manual.
That image carries the hero state (undo arrow active, redo arrow
dim), so it already *visually* documents the undo / redo pair — but
the narrative and the caption skipped them entirely. Fixed in this
commit:

- **Caption** (`generate_manual.js` line ~968) — extended to mention
  "undo / redo arrows" alongside title, preset navigation, OSC toggle,
  Output Format, and Algorithm/HRTF.
- **Narrative** (`generate_manual.js` lines ~970–982) — bolded
  "undo / redo arrows" inserted between preset navigation and the OSC
  toggle (matching physical header order), with a one-line gloss:
  *step backwards or forwards through recent parameter changes —
  dimmed when the corresponding history is empty*.

`docs/wiki/controls-reference.md` had a parallel gap — the Header Bar
table enumerates every other control (Preset Name, Prev/Next Arrows,
Save, OSC RECV, Output Format, Algorithm / HRTF Profile, SML Badge)
but not the undo/redo pair. Fixed by adding an **Undo / Redo Arrows**
row between Save and OSC RECV, noting that each arrow is dimmed when
its history is empty (undo activates after the first parameter
change, redo activates only after an undo).

**Asset refinement:** `screenshot_header.png` is produced as a PIL
crop of `screenshot.png` at `(0, 0, 1640, 104)` — that bound was
already correct, but the comment in
`scripts/regenerate_screenshots.sh` did not say so, making the
relationship to the header/map divider (drawn at scaled y=103,
native y=52 via `kHeaderHeight`) unclear for future editors. Added a
one-line note to the crop tuple explaining that the bound ends at the
divider. No pixel change to the PNG.

**Files in this commit:**

- `docs/generate_manual.js` — caption + narrative additions.
- `docs/wiki/controls-reference.md` — new Undo / Redo Arrows table row.
- `scripts/regenerate_screenshots.sh` — clarifying comment on the
  header-crop PIL tuple.
- `docs/SCREENSHOT_REVIEW_ROUND_3.md` — this tracker.

## Item 18 — Hero video (Merry-Go-Round)

**New deliverable — not a review fix.** Replaces the static
`screenshot_full.png` in the `andrewrahman-com` homepage hero
(`app/page.tsx:257`) with a short, muted, seamlessly-looping video of
OpenSpatialDelay running a spatially animated preset. Goal: sell
"every echo, somewhere in the room" on first paint.

### Locked decisions

- **Aspect / size.** Matches the current hero image exactly:
  **1640 × 1160** (1.414:1). Anything else causes CLS; anything
  smaller upscales. Capture at native 2× retina (plugin window is
  820 × 580 at 1×).
- **Preset.** `Merry-Go-Round` (preset #64, `Source/PresetData.cpp:1053`).
  8 taps on Circle trajectories, 45° apart, rate 0.20 Hz → **one full
  rotation = 5 seconds**. Two rotations = **10 s**, a natural loop at
  the upper bound of the spec's 6–10 s window. Endpoints match
  automatically because the motion is periodic; no encoder hack
  needed.
- **Cursor.** Hidden during capture.
- **Frame.** Plugin window only — no DAW chrome, no wallpaper, no
  title bar decoration if avoidable.
- **Audio.** Muted on output. Capture audio is only needed if we want
  meter LEDs to pulse visibly; if we keep audio running into the
  plugin, pick a source whose period divides 5 s so the meters loop
  cleanly too (e.g. a 2.5 s or 5 s loop at any tempo). Otherwise
  silence the input and rely on the spatial-map rotation to carry
  the motion.
- **Spec compliance.** 30 fps, SDR (Rec.709), no audio track,
  `+faststart`, AV1 target ≤2 MB and H.264 fallback ≤4 MB — as given
  by the user.

### Workflow (three handoff points)

**Step A — Capture (HITL).** Andrew only. Produces
`docs/assets/hero-source.mov` (gitignored until approved; large file).

1. Open a DAW with a loopable input (either silent or a 5 s loop).
2. Load OSD v1.0, select `Merry-Go-Round`, let the taps settle
   (one full rotation ≈ 5 s).
3. Resize the plugin window to its native 820 × 580 size. Hide the
   cursor before starting the capture.
4. Screen-record the plugin window only, at 2× retina (recorded
   region = 1640 × 1160 physical pixels), 60 fps, ProRes 422 or
   HEVC-max-quality, no audio track. Record at least **three full
   rotations** (≥15 s) so we have headroom to trim two clean
   rotations.
5. Export as `docs/assets/hero-source.mov`.

Options for the recording tool, in order of simplicity:

- **Cmd + Shift + 5 → Record Selected Window** (macOS built-in).
  Hide cursor via the Options menu. Saves `.mov`.
- **QuickTime Player → File → New Screen Recording.** Same engine,
  marginally different controls.
- **`ffmpeg -f avfoundation ...`** if we need to script it, but
  overkill for a one-off hero.

**Step B — Trim to exact loop (automated, 1 command).** Identify the
first frame of a clean rotation (any moment on the cycle is fine —
pick one where no transient is masking the tap markers). Trim to
exactly 10 s from that frame:

```
ffmpeg -ss <start-in-seconds> -i hero-source.mov -t 10 -c copy hero-trimmed.mov
```

Verify the loop: extract the first and last frame and diff them. They
should be visually identical (the trajectory rate is deterministic):

```
ffmpeg -ss 0       -i hero-trimmed.mov -frames:v 1 first.png
ffmpeg -sseof -0.034 -i hero-trimmed.mov -frames:v 1 last.png
compare -metric AE first.png last.png diff.png   # ImageMagick
```

If the diff is visible (non-periodic audio-driven meters at the
seam), re-capture with silent input or shorten audio loop.

**Step C — Encode + publish (automated).**

```
# WebM / AV1 — primary, target ≤2 MB
ffmpeg -i hero-trimmed.mov -c:v libsvtav1 -preset 6 -crf 32 \
  -pix_fmt yuv420p -an -movflags +faststart \
  andrewrahman-com/public/assets/hero-plugin.webm

# MP4 / H.264 — fallback, target ≤4 MB
ffmpeg -i hero-trimmed.mov -c:v libx264 -preset slow -crf 23 \
  -profile:v high -level 4.0 -pix_fmt yuv420p -an \
  -movflags +faststart \
  andrewrahman-com/public/assets/hero-plugin.mp4
```

Verify file sizes against budgets. If over budget, bump CRF by 2–4
and re-encode. If under budget by a wide margin, lower CRF for extra
quality.

**Step D — Wire into the hero (automated).** In
`andrewrahman-com/app/page.tsx` around line 257, swap the `<Image>`
for a `<video>` element that preserves the 1640 × 1160 box and keeps
`screenshot_full.png` as the poster (covers first-paint before the
video loads, and covers prefers-reduced-motion users):

```tsx
<video
  autoPlay
  muted
  loop
  playsInline
  preload="metadata"
  poster="/assets/screenshot_full.png"
  width={1640}
  height={1160}
  className="w-full h-auto block"
  aria-label="OpenSpatialDelay plugin window — eight delay taps rotating in a 3D halo around the listener (Merry-Go-Round preset)"
>
  <source src="/assets/hero-plugin.webm" type="video/webm" />
  <source src="/assets/hero-plugin.mp4"  type="video/mp4" />
</video>
```

Also:
- Add a `@media (prefers-reduced-motion: reduce)` rule in the hero
  section CSS that sets `video { display: none; }` and falls back to
  the still `screenshot_full.png` via a sibling `<img>`. A11y.
- Keep the `BorderBeam` sibling wrapping the video — same effect
  applies.
- Run the site's Playwright hero test with the new markup (expect
  updates to any `<Image>`-specific assertions).

### Open gotchas

- **Capture resolution vs. render scale.** Confirm the selected-window
  capture actually writes a 1640 × 1160 pixel buffer, not 1640 × 1160
  at 1× (which is a 2× upscale of 820 × 580 — blurry). macOS Cmd+Shift+5
  captures at retina native by default on HiDPI displays, so on a
  Retina-mode external it should Just Work, but worth a quick
  `sips -g pixelWidth hero-source.mov` check.
- **Bitrate budget.** 10 s @ 1640 × 1160 @ 30 fps is chunky. If the
  AV1 encode blows past 2 MB even at CRF 36, the fallback plan is to
  drop to one rotation (5 s) or scale to 1312 × 928 (still >1280
  minimum guideline). Don't drop below 1600 wide per the spec.
- **Loop cleanliness with audio.** If we keep audio running so the
  meters pulse, the meter LEDs add a non-periodic signal to the frame
  diff at the seam. Two options: silence the input (cleanest loop,
  but meters are dark), or use an audio loop whose period divides
  5 s (e.g. 2.5 s or 5 s at any tempo; bar length should compute to
  an exact multiple).

### HITL checklist (for Andrew's capture session)

- [x] Plugin window resized to 820 × 580 native
- [x] Merry-Go-Round preset loaded and left running long enough to
      stabilise
- [x] Cursor hidden
- [x] DAW chrome out of frame
- [x] Audio decision made (meters pulse; loop verified periodic)
- [x] Screen recording, ≥15 s, no audio track
- [x] Source delivered as `~/Desktop/merry-go-round.mov` (1636 × 1156,
      H.264, 14.985 s, 29 fps avg, 6.14 MB)

### As-built (2026-04-20)

Source arrived at 1636×1156, **VFR (avg 29.02 fps, durations varying
0.020–0.050 s)**, 434 frames, 14.985 s, H.264, 6.14 MB.

**First attempt (rejected):** normalised to 1640×1160 / 30 fps CFR /
10 s via `-vf scale,fps=30`. User flagged visible stutter at loop —
the `fps=30` filter duplicated source frames to hit 30 fps CFR, and
every duplicated frame registers as a motion-pause.

**Revised pipeline (ships):** preserve source VFR timing, no rate
conversion, no trim — all 434 source frames pass through 1:1. The
full 14.97 s source *is* the prepared loop; any shorter trim creates
a sub-frame motion jump at the wrap because no source frame PTS
lands exactly at t=10.000 s.

```bash
# Master: scale only, keep VFR, keep frame count, near-lossless
ffmpeg -y -i ~/Desktop/merry-go-round.mov \
  -vf "scale=1640:1160:flags=lanczos" \
  -pix_fmt yuv420p -c:v libx264 -preset veryslow -crf 14 -an \
  -fps_mode passthrough -movflags +faststart \
  docs/assets/hero-video-build/hero-master.mov

# Primary — WebM / AV1
ffmpeg -y -i hero-master.mov -c:v libsvtav1 -preset 6 -crf 32 \
  -pix_fmt yuv420p -an -fps_mode passthrough -movflags +faststart \
  ../hero-plugin.webm

# Fallback — MP4 / H.264 (no explicit -level; auto-selects 5.1 —
# source's tagged r_frame_rate=120 trips level-4.0's MB-rate check
# even though actual frames average 29 fps; 5.1 is universally
# supported by modern browsers)
ffmpeg -y -i hero-master.mov -c:v libx264 -preset slow -crf 23 \
  -profile:v high -pix_fmt yuv420p -an \
  -fps_mode passthrough -movflags +faststart \
  ../hero-plugin.mp4
```

**As-built artefacts (in `docs/assets/`):**

| File | Codec | Size | Budget | Headroom | Frames |
|------|-------|------|--------|----------|--------|
| `hero-plugin.webm` | AV1 (libsvtav1, CRF 32) | **396 KB** | ≤2 MB | 80% | 434 |
| `hero-plugin.mp4` | H.264 High @ L5.1 (CRF 23) | **574 KB** | ≤4 MB | 86% | 434 |

Both: 1640 × 1160, **VFR avg 29.02 fps** (source preserved), yuv420p,
no audio, `+faststart`, 14.97 s duration. Intermediate master
`hero-video-build/hero-master.mov` (~1.1 MB) retained for
re-encoding. Recommend adding `docs/assets/hero-video-build/` to
`.gitignore`.

**Spec deviations (intentional, documented):**

- **Duration 14.97 s** — spec said 6–10 s. Kept entire source to
  preserve 1:1 frame count after user flagged stutter from CFR
  resampling. Any sub-15 s trim would introduce a sub-frame motion
  jump at the loop wrap because no source-frame PTS aligns with
  t=10.000 s. File sizes remain well under budget even at 14.97 s.
- **Frame rate 29.02 fps VFR** — spec said 30 fps. Source was
  captured VFR and any upconversion to 30 fps CFR introduces
  duplicate frames → visible stutter. VFR is well-supported on all
  modern browsers; the "30 fps" spec target was a bitrate-budgeting
  recommendation, not a playback requirement.
- **H.264 Level 5.1** — spec said 4.0. Level 4.0 MB-rate cap
  (245 760 MB/s) was blown by the source's container-tagged 120 fps
  reference, even though actual frames average 29 fps. Level 5.1 is
  universally supported and imposes no real constraint at this
  resolution/frame-rate.

### Still to do

- [ ] Copy `hero-plugin.webm` + `.mp4` to
      `andrewrahman-com/public/assets/`.
- [ ] Swap `<Image src="/assets/screenshot_full.png" …>` at
      `app/page.tsx:257` for a `<video>` element with dual `<source>`,
      `poster="/assets/screenshot_full.png"`, and width/height
      matching 1640 × 1160 to preserve the existing reserved box.
- [ ] Add `prefers-reduced-motion` fallback (show poster instead of
      playing the video for users who've opted out).
- [ ] Run site Playwright hero tests; update any
      `<Image>`-specific assertions to the new element.
- [ ] Add `docs/assets/hero-video-build/` to `.gitignore`.

## Next session / next item

Items 7, 12, 16, 17 are closed. Item 10 is closed via HITL (see
above). Item 18 encodes are complete; site wiring is the remaining
step. Remaining open items: **11** (preset-menu screenshot) and
**18** (hero video — site wiring only). Item 11 is HITL-eligible
like item 10 — if mock iteration exceeds attempt 2, escalate to live
capture following the item-10 pattern. When items 11 and 18 close,
round 3 is complete and the branch can roll up to issue #168.

### Session backlog (beyond round 3)

Captured during round-3 work, not tied to any specific issue #168 item:

- **Signal-flow diagram rework.** `.context/attachments/signal-flow.png`
  has "several issues" (per user) that need heavy revision. Not started;
  issue list not yet captured — the user will describe when we return.
- **Trajectory screenshots.** Prior attempt lives in
  `docs/assets/trajectories/` (14 files: `traj_bounce.png`, `traj_circle.png`,
  `traj_cross.png`, `traj_figure8.png`, `traj_heart.png`, `traj_helix.png`,
  `traj_infinity.png`, `traj_line.png`, `traj_none.png`, `traj_orbit.png`,
  `traj_random.png`, `traj_spiral.png`, `traj_square.png`,
  `traj_triangle.png`). Per user those are incorrect and the set needs
  to be regenerated. The new `preset-showcase` machinery
  (`tools/screenshot_tool.cpp` `pumpTrajectories()` +
  `setDrawFullTrajectoryForScreenshot`) is the right starting point —
  each trajectory shape needs its own capture with the shape enabled,
  the tap selected, and the trail rendered.

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
