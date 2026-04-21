# Screenshot Review — Round 2 (Issue #168)

This is a task handoff for the next session working on issue #168
screenshot revisions. Prior context: commits `b6b061d` (initial tool +
orchestrator) and `f41a8d9` (first-pass revisions). The user reviewed
the 18 regenerated screenshots from `f41a8d9` and posted the findings
below.

- **Branch**: `AndrewRahman/screenshot-fixes`
- **Base commit**: `f41a8d9`
- **User's review comment**:
  https://github.com/Spatial-Media-Lab/OpenSpatialDelay/issues/168#issuecomment-4268638612
- **User reference screenshots** (for items 10 and 11):
  - `.context/attachments/Screenshot 2026-04-17 at 15.50.02.png` — real
    output-format combo box popup
  - `.context/attachments/Screenshot 2026-04-17 at 15.50.40.png` — real
    preset menu with submenu

## Accepted as-is (no work required)

Items 1, 3, 4, 5, 8, 9, 13, 15, 18 from the review. Don't touch the
corresponding screenshots or the capture modes that produce them:
`screenshot.png` (see item 7 below — it will be *replaced* by a
showcase capture), `screenshot_bottom_panel.png`, `screenshot_drawer.png`,
`screenshot_elevation.png`, `screenshot_header.png`,
`screenshot_osc_section.png`, `screenshot_save_preset.png`,
`screenshot_spatial_map.png`, `screenshot_wobble.png`.

## Revisions to execute

### Item 2 — `screenshot_annotated.png`: colour-by-section callouts

Current state: every callout is flat amber. Reads pixelated vs. the
rest of the UI and doesn't group related UI regions visually.

Do:
- Assign a **section colour** to each callout. Sections correspond
  roughly to the groups in `CALLOUTS[]` in
  `docs/assets/annotate_screenshot.py`:
  - Header row (SML, preset browser, undo/redo, format dropdowns,
    profile)
  - Spatial map + global drawer
  - Right-panel sections (DELAY / MOD / TONE / MIX / OSC)
  - Bottom panel (tap selector, per-tap controls, trajectory)
- Break the **legend** into subsections that mirror the callout colour
  groups. Each subsection should have a short heading (e.g. "HEADER",
  "MAP", "RIGHT PANEL", "PER-TAP") and its items should use the
  matching hue.
- Pick 4–5 hues that stay high-contrast against both the dark
  background and the plugin's cyan / purple accent colours. Keep dark
  text on every circle (existing approach).

### Item 6 — `screenshot_elevation_map.png`: flip order + use real label

Current state: 12-tap spiral with +90° at Tap 1 and −90° at Tap 12; the
tool overlays elevation labels via `juce::Graphics` after snapshot.

Do:
1. **Flip the spiral** so Tap 1 = −90° and Tap 12 = +90°. Change the
   elevation formula in the `elevation-map` mode of
   `tools/screenshot_tool.cpp` from `90.0f - t * 180.0f` to
   `-90.0f + t * 180.0f`. Keep the azimuth sweep and distance ramp
   unchanged (or adjust if the visual balance looks wrong once
   flipped).
2. **Drop the tool-drawn overlay labels** and use the plugin's own
   per-tap elevation label instead. The label currently only renders
   next to the **selected** tap; see `SpatialMapComponent::paint` in
   `Source/PluginEditor.cpp` around the existing "elevation readout"
   code path (grep for the text-rendering call that uses
   `objects[(size_t)selectedObject].elevationDeg`).

   Three options, pick the least invasive:
   - (a) Screenshot-mode flag on `SpatialMapComponent` that causes the
     paint routine to render the elevation label for every *enabled*
     tap (not just the selected one). Cheapest and keeps rendering in
     one place.
   - (b) Loop in the tool: set `selectedObject = i`, snapshot each tap
     separately, composite. More complex, risks re-entrancy.
   - (c) Expose the plugin's label-draw routine and call it from the
     tool per-tap. Duplicates code.

   Read the existing paint code first and pick based on how parameterised
   it already is.
3. The label must appear **above or below** the tap dot in the **tap's
   colour** — exactly matching how Tap 1 currently renders (visible in
   the current output).

### Item 7 — `screenshot.png`: feature-showcase state

Current state: `screenshot.png` is the first-launch / default editor
with Quad Ping-Pong at baseline.

Do: replace it with a capture where every major feature is engaged.
Add a new capture mode (`--mode full-showcase` or similar) that
programmatically drives the editor into this state before snapshotting:
- Output format = **9.1.6**
- Algorithm = **VBAP**
- **≥8 taps enabled**, varied azimuth / elevation / distance
- **≥1 trajectory** visibly drawn on the spatial map (trajectory shape
  set to something other than None — Arc, Orbit, or similar — with a
  trail currently on-screen)
- **OSC Send enabled** (not Receive — the current state has Receive on)
- **AIR on** (MIX section toggle)
- **FLT on** (TONE section toggle) with the filter graph showing
  min-Q one side / max-Q the other
- **MOD on** with Amount + Morph knobs at visibly non-zero positions
- **Sync on** in the DELAY section, **Triplet** enabled

This showcase state doubles as the source for items 12, 16, and the
re-run of `annotate_screenshot.py`. The callout coordinates in
`annotate_screenshot.py` probably need re-tuning once the source image
changes — check every callout and nudge any that drift off-target.

### Item 10 — `screenshot_output_menu.png`: match real combo box

Current state: mock shows the flat list of 23 formats with the current
one ticked.

Real behaviour (per user's reference attachment): the real
`juce::ComboBox` popup duplicates the currently-selected item at the
top, followed by a separator, then the full flat list.

Do — preferred approach: drive the **real** popup via
`outputFormatBox.showPopup()` and snapshot once the popup has rendered.
Headless popup capture needs a few frames of message-loop pumping —
search JUCE forums / docs for prior art on capturing `PopupMenu`
snapshots in tests.

Fallback approach: update `buildOutputDropdownMock` in
`tools/screenshot_tool.cpp` to prepend: current item (ticked) +
separator + full flat list with current item ticked again.

### Item 11 — `screenshot_preset_menu.png`: match real popup + Merry-Go-Round

Current state: mock shows parent folder list + hovered category
submenu, with "Classic Delays" as the hovered category.

Real behaviour (per user's reference attachment): parent list shows a
tick ✓ on the currently-active category (Spatial Movement in the
reference), each category has a right-arrow submenu indicator, and the
submenu appears to the right with a tick on the active preset
(Merry-Go-Round).

Do:
1. Change `MAIN_PRESET` in `scripts/regenerate_screenshots.sh` to
   **Merry-Go-Round** for the preset-menu capture (this also means the
   spatial map shows the Merry-Go-Round trajectory in that screenshot
   — good).
2. Rework the preset-menu mock in `tools/screenshot_tool.cpp` to match
   the real popup visually: tick column on the parent list showing
   which category is active, arrow indicators on each parent row,
   submenu positioned flush with the parent's right edge, tick on the
   active preset.
3. Ideally drive the real menu via JUCE's `PopupMenu::showMenuAsync`
   and pump the loop — same investigation path as item 10.

### Item 12 — `screenshot_right_panel.png`: every section engaged

Current state: DELAY / MOD / TONE / MIX with all controls at default
(MOD and TONE toggles off, graphs greyed out).

Do: enable everything. Requirements overlap with item 7 — if item 7
lands first, re-use the same `full-showcase` state for this crop. The
right-panel crop should show:
- DELAY with Sync **on** and Triplet **on** (check the DELAY section
  layout to find the Triplet control; it's currently a hidden/toggle
  mode of the note-division selector)
- MOD toggle **on** with Amount and Morph knobs at visible non-zero
  positions
- TONE / FLT toggle **on**; filter graph shows a clear curve (min-Q on
  one end, max-Q on the other)
- MIX / AIR toggle **on**

The crop bounds (x 1112–1640, y 104–1018) are already correct; no
layout changes needed.

### Item 16 — `screenshot_tone_section.png`: tighten + feature-on

Current state: filter graph with FLT off (greyed out) and generous
top/bottom padding.

Do:
- Tighten `getToneSectionBoundsForScreenshot()` vertical padding —
  current is 12px top, 12px bottom; try **6px top, 6px bottom** (keep
  10px L/R for breathing room).
- Ensure FLT is **enabled** in the preset/state used for this capture
  (re-use the `full-showcase` state from item 7 once available).
- Filter must show **min-Q one side, max-Q the other** — this is the
  same requirement as items 7 and 12.

### Item 17 — `screenshot_undo_active.png`: undo only, redo inactive

Current state: `populateUndoHistory()` in
`tools/screenshot_tool.cpp` ends with
`processor.performInternalUndo()`, which leaves *both* arrows active
(the previous spec).

Do: remove the trailing `performInternalUndo()` call so only the undo
arrow is active and the redo arrow is inactive. Update the
accompanying comment block (and the file-level mode docs) to reflect
the corrected behaviour.

## Investigation only (no fix)

### Item 14 — where is `screenshot_shimmer.png` referenced?

The user is asking whether this screenshot is still used anywhere.
Before the next review pass, grep the repo for every reference:

```
grep -R "screenshot_shimmer" --include="*.md" --include="*.docx" \
    --include="*.html" --include="*.json" --include="*.js" .
```

Check especially:
- `README.md`
- `docs/` (MANUAL.docx, MANUAL_UPDATE_PLAN.md, release notes)
- `docs/plugin-proposals/`
- `docs/research/`
- Website repo (sibling `andrewrahman-com`) if the screenshot is
  syndicated externally

Report the findings in the next review comment; do not remove the
screenshot until the user decides.

## Workflow

1. Read this whole file before starting.
2. Read the user's GitHub comment:
   `gh issue view 168 --comments | tail -60` — the salient comment is
   dated 2026-04-17 and titled "Second-pass review".
3. View the two reference screenshots in `.context/attachments/` so you
   know what the real popups look like.
4. Work through the revisions in any order — they are independent.
   Prefer landing item 7 (full-showcase) early because items 12 and 16
   reuse its state.
5. Re-run `bash scripts/regenerate_screenshots.sh`.
6. Read every revised PNG back via the `Read` tool to verify.
7. Commit everything (code + regenerated PNGs) in one commit.
8. Push to `AndrewRahman/screenshot-fixes`.
9. Report back on the issue thread with a summary of what changed and
   ask for a third review pass.

## What NOT to do

- Don't rename the branch — it's already `AndrewRahman/screenshot-fixes`.
- Don't touch the 9 accepted screenshots unless a capture-mode change
  incidentally re-runs them (that's fine — their content will be
  identical to `f41a8d9`).
- Don't try to fix `signal-flow.png` — that's a separate track
  (resolved in pass 4, merged via PR #211 / PR #213).
- Don't delete `screenshot_shimmer.png` or any other file without the
  user's explicit go-ahead (item 14 is investigation-only).
- Don't run `killall AudioComponentRegistrar` or `sudo` anything —
  standing project rules in `CLAUDE.md`.
