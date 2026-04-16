---
phase: 03-personal-website
plan: 01
subsystem: ui
tags: [nextjs, tailwind, playwright, copywriting, marketing-content, tdd]

# Dependency graph
requires:
  - phase: 03-personal-website/00
    provides: "Wave 0 validation infra — tests/asset-existence.spec.ts references /assets/sml-logo.svg (Plan 04 lands SVG file itself); homepage-content.spec.ts baseline extended here with +5 assertions for the new marketing strings"
provides:
  - "Hero stat <dl> rewritten to 4 locked v1.0.0 stats (12 Delay Taps, 7 Spatialization Algorithms, 5 HRTF Profiles +1 Simple, 70 Factory Presets) with +1 Simple footnote as <sup>"
  - "FEATURES array rewritten 5→6 cards in D-03 order with tap indices 0/5/7/9/3/11; card 6 image path updated to /assets/sml-logo.svg (D-11)"
  - "Hero headline 3-span structure preserved with new plain-language copy; adjacent sub-paragraph rewritten to drop phase-vocoder Doppler jargon (Rule 2 scope extension)"
  - "DownloadCTA headline rewritten with explicit triple-value structure (free / free forever / open-source)"
  - "Features section subhead rewritten from 'Five things' to 'Six capabilities'"
  - "5 new Playwright content-regression assertions: locked hero stats, retired-stats-gone, Simple footnote, 6-card count, Five-things-removed"
  - "ADM-OSC claim verified against Source/PluginProcessor.cpp: both Send (line 1631+) and Receive (line 5470+) are implemented — 'OSC in/out (ADM-OSC)' parenthetical kept"
affects: [03-02, 03-03, 03-04, 03-05, 03-06, 03-07, 03-08]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Inline <sup> footnote on hero <dd> stat labels — renders adjacent to label, font-mono-osd 10px, text-dim, with aria-label for screen readers"
    - "FEATURES array data-driven rewrite preserves the renderer contract — executor mutates entries only, never the map() body, keeping downstream stable"
    - "Playwright negative assertions (not.toContainText) as regression guards for copy decisions (D-02, D-03)"

key-files:
  created:
    - ".planning/phases/03-personal-website/03-01-SUMMARY.md"
  modified:
    - "../andrewrahman-com/app/page.tsx (Hero stat <dl>, FEATURES array 5→6, Features subhead, Hero 3-span headline, Hero sub-paragraph, DownloadCTA headline)"
    - "../andrewrahman-com/tests/homepage-content.spec.ts (+5 new tests; 7→12 total)"

key-decisions:
  - "ADM-OSC claim for feature card 5 is TRUTHFUL — verified via Source/PluginProcessor.cpp lines 1592–1666 (Send, 30Hz broadcast, /adm/obj/N/aed) and 5470+ (Receive, accepts /adm/obj/N/ and /osd/obj/N/ namespaces). Card 5 title kept as 'OSC in/out (ADM-OSC)' with parenthetical."
  - "Tap index collision avoidance — used tapIndex 9 for card 4 (algorithms, purple #A667E4) and tapIndex 11 for card 6 (open-source, magenta #E467E4) per UI-SPEC §Component Inventory recommendation to avoid tap-9/10 purple collision on the feature-card strip."
  - "Hero sub-paragraph 'phase-vocoder Doppler engine' language retired (Rule 2 scope extension): violates D-04 (Doppler dropped from hero slate), UI-SPEC constraint (no FFT sizes/paper names on public surfaces), and user memory rule (DSP jargon is 'pointlessly complex')."
  - "Final hero headline copy: Line 1 'OpenSpatialDelay —' (fixed) / Line 2 'a 3D spatial delay where every echo lives somewhere in the room.' / Line 3 italic 'VST3 + AU. macOS + Windows. Free, open-source.'"
  - "Final DownloadCTA headline: 'Download OpenSpatialDelay — free, free forever, and open-source.' (triple-value structure preserved)."

patterns-established:
  - "Capability-level marketing copy: all 6 feature-card bodies + hero + CTA copy stays at user-visible capability level. No algorithm paper names (Laroche–Dolson), no FFT sizes (2048), no internal mode names (Woodworth). This pattern carries forward to all downstream phase-3 plans that touch public page.tsx strings."
  - "Footnote <sup> pattern inside hero stat <dd>: conditional render gated on label string, font-mono-osd tiny with tracking-[0.1em] normal-case, aria-label for AT — reusable for any future stat that needs an asterisk."
  - "ADM-OSC verification checkpoint: any public-facing claim about external protocol support must be grep-verified against Source/ before shipping. Cross-repo copy-correctness gate."

requirements-completed: [WEB-01]

# Metrics
duration: 5min
completed: 2026-04-16
---

# Phase 3 Plan 01: Hero Stats + 6 Feature Cards Summary

**Hero stat <dl> locked to 12/7/5/70 with +1 Simple footnote, FEATURES rewritten 5→6 cards in D-03 order, hero + Download CTA headlines polished to producers-first/capability-level copy — 5 new Playwright regression assertions pin the marketing thesis in place.**

## Performance

- **Duration:** 5 min
- **Started:** 2026-04-16T21:56:31Z
- **Completed:** 2026-04-16T22:01:45Z
- **Tasks:** 3 of 3
- **Files modified:** 2 (both in sibling repo `../andrewrahman-com/`)
- **Tests added:** 5 (12 homepage-content tests total; 4 smoke tests unchanged; all green)

## Accomplishments

- Hero stat grid swapped from `[5 HRTF profiles, ±12 semitones, 2048 sample FFT, GPL-3.0 forever free]` → `[12 Delay Taps, 7 Spatialization Algorithms, 5 HRTF Profiles (+1 Simple), 70 Factory Presets]` (D-01).
- FEATURES array rewritten from 5 cards (Each echo/Phase-vocoder Doppler/Trajectory/Binaural-first/Free+OSS) to 6 cards per D-03 (12 taps / Trajectory / Measured HRTFs / 7 algorithms / OSC / Free+OSS).
- Features section subhead changed from "Five things, in order of how surprising they feel..." to "Six capabilities that make OpenSpatialDelay a spatial instrument, not a stereo delay with a pan knob."
- Hero 3-span headline rewritten with 3-line structure intact; retired the "a 3D spatial delay for VST3 + AU / on macOS & Windows" split in favour of a capability claim on line 2 and a platform+positioning kicker on line 3.
- Adjacent hero sub-paragraph rewritten to eliminate "phase-vocoder Doppler engine" DSP jargon (Rule 2 scope extension — see Deviations below).
- DownloadCTA headline rewritten from "Get OpenSpatialDelay — free, forever, open-source." → "Download OpenSpatialDelay — free, free forever, and open-source." (triple-value structure preserved).
- Card 6 `image` path swapped from `sml-logo.png` → `sml-logo.svg` (D-11) — the SVG file itself lands in Plan 04.
- ADM-OSC claim on feature card 5 verified truthful against Source/PluginProcessor.cpp before shipping the parenthetical.
- 5 new Playwright content-regression tests added (positive hero stats / negative retired stats / footnote / 6-card count / Five-things absence); all 12 homepage-content tests + 4 smoke tests green.

## Task Commits

Each task was committed atomically in the sibling site repo `../andrewrahman-com/`:

1. **Task 1: Rewrite Hero stat <dl> + 3 homepage-content assertions** — `67801e3` (feat)
2. **Task 2: Rewrite FEATURES 5→6 cards + subhead + 2 assertions** — `4073142` (feat)
3. **Task 3: Rewrite Hero headline + DownloadCTA headline** — `39122d0` (feat)

**Plan metadata:** pending — created by final-commit step below (SUMMARY.md + STATE.md + ROADMAP.md + REQUIREMENTS.md in the plugin repo).

## Files Created/Modified

### Sibling repo `/Users/andrewrahman/conductor/repos/andrewrahman-com/`

- `app/page.tsx` — 4 edits total: (1) Hero stat inline tuple array + `+1 Simple` <sup> footnote inside the map-callback; (2) FEATURES array 5→6 entries with new tap indices; (3) Features subhead `<p>`; (4) Hero 3-span headline + adjacent sub-paragraph; (5) DownloadCTA headline.
- `tests/homepage-content.spec.ts` — 5 new tests inside existing describe block; file grew from 68 lines / 7 tests to 103 lines / 12 tests.

### Plugin repo `/Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/`

- `03-01-SUMMARY.md` (NEW, this file).

## Decisions Made

- **ADM-OSC verdict (Task 2 Step D):** claim is verified truthful. Both Send (30Hz polling, `/adm/obj/N/aed` namespace, PluginProcessor.cpp line 1631+) and Receive (accepts `/adm/obj/N/` + `/osd/obj/N/` + `/osd/global/` namespaces, line 5470+) are implemented in v1.0.0. Card 5 title kept as "OSC in/out (ADM-OSC)" with parenthetical intact.
- **Tap index choice for card 4 vs card 6:** tap 9 (#A667E4 purple) for algorithms / tap 11 (#E467E4 magenta) for open-source, per UI-SPEC §Component Inventory direction to avoid tap-9/10 purple collision.
- **Final hero headline (3 spans):** `OpenSpatialDelay —` / `a 3D spatial delay where every echo lives somewhere in the room.` / italic `VST3 + AU. macOS + Windows. Free, open-source.`
- **Final hero sub-paragraph:** "Twelve delay taps, each with its own position on the sphere, pitch, and trajectory. Binaural on headphones, multichannel-ready for film, VR, and live spatial work."
- **Final Features subhead:** "Six capabilities that make OpenSpatialDelay a spatial instrument, not a stereo delay with a pan knob."
- **Final DownloadCTA headline:** "Download OpenSpatialDelay — free, free forever, and open-source."

## Test Pass Count

- `tests/homepage-content.spec.ts`: **12 passed** (7 pre-existing + 5 new; `npm run build` + `npx playwright test` on commit `39122d0`).
- `tests/smoke.spec.ts`: **4 passed** (unchanged).
- Combined run in the final verify step: **16 passed** in 2.2s (2 workers).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 — Missing Critical] Dropped "phase-vocoder Doppler engine" from hero sub-paragraph**

- **Found during:** Task 3 (Hero headline rewrite)
- **Issue:** The Task 3 <action> scope named the 3-span headline + DownloadCTA, but while rewriting the adjacent `<p>` was untouched it still contained `"Free, open-source (GPL-3.0). Each echo lives at its own position in 3D space — twelve taps, five HRTF profiles, a phase-vocoder Doppler engine."` This directly violates:
  - D-04 (phase-vocoder Doppler card dropped from hero slate)
  - UI-SPEC §Hero headline constraint ("No FFT sizes, no algorithm paper names, no 'STFT'")
  - User memory rule "Public OSD surfaces stay at capability level; DSP jargon (FFT size, paper names) is 'pointlessly complex'"
- **Fix:** Rewrote the paragraph to `"Twelve delay taps, each with its own position on the sphere, pitch, and trajectory. Binaural on headphones, multichannel-ready for film, VR, and live spatial work."` — keeps the capability pitch, lands the 12-taps claim (reinforces hero stat 1), frames headphones + multichannel use-cases (producers + sound-designers), no DSP internals.
- **Files modified:** `../andrewrahman-com/app/page.tsx` (Hero section, same function as the 3-span edit)
- **Verification:** All 16 homepage-content + smoke tests still green post-edit; `grep -q "phase-vocoder Doppler" app/page.tsx` returns 1 (no match).
- **Committed in:** `39122d0` (Task 3 commit, explicitly noted in commit body as "Scope extension (Rule 2)").

---

**Total deviations:** 1 auto-fixed (1 missing critical)
**Impact on plan:** The fix is essential for marketing-copy correctness and aligns with an explicit user-memory rule recorded as a hard constraint. No scope creep — the edit touched a paragraph directly adjacent to the 3-span block Task 3 already modified.

## Issues Encountered

- **Playwright static-build caching:** Initial RED/GREEN runs after page.tsx edits needed `npm run build` + a `pkill -f "serve out"` to flush the cached `npx serve out` worker (per `playwright.config.ts`: `reuseExistingServer: !process.env.CI`). Ran `npm run build` once per task before the GREEN check. Noted for Plan 02/03 executors who'll also edit `app/page.tsx`.
- **Plan acceptance-criterion false negative:** The plan spec `grep -c "Download OSD →" app/page.tsx` returned 0 because the source separates the label text from its arrow into two elements (`Download OSD` + `<span aria-hidden>→</span>`). This is pre-existing structure, not a regression. `grep -c "Download OSD"` correctly returns 2. The Playwright assertion `CTAs appear at top and bottom of the page` (≥2 Download links) already pins this at runtime; no code change needed.

## User Setup Required

None — all automation runs without external service configuration.

## Next Phase Readiness

- **Plan 03-02 (next wave-1 plan)** is unblocked. Per plan header `<context_note>`: Plans 01/02/03 all edit `app/page.tsx` serially (02 depends_on [00, 01], 03 depends_on [00, 01, 02]). My commits (`67801e3`, `4073142`, `39122d0`) leave `app/page.tsx` in a clean state — FEATURES and Hero are finalised; Plan 02 can now touch any other section without merge risk.
- **Plan 03-04 (asset swaps)** — card 6 image path is already `sml-logo.svg`; Plan 04 only needs to copy the SVG file from Downloads into `public/assets/sml-logo.svg` (D-11) and the feature card will light up.
- **Plan 03-06 (a11y audit)** — the new `<sup>` footnote on the HRTF Profiles stat has an `aria-label="plus one Simple mode for low CPU"` for screen-reader parity. Plan 06 will confirm the label reads cleanly under axe.

## Self-Check: PASSED

Verified post-write:

- `test -f /Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/03-01-SUMMARY.md` → FOUND (written via Write tool)
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 67801e3` → FOUND: "feat(03-01): rewrite hero stat <dl> to 4 locked v1.0.0 stats (D-01)"
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 4073142` → FOUND: "feat(03-01): rewrite FEATURES 5→6 cards + update subhead (D-03)"
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 39122d0` → FOUND: "feat(03-01): rewrite hero headline + DownloadCTA headline (B-3)"
- All 12 homepage-content tests + 4 smoke tests passed in final run.

---
*Phase: 03-personal-website*
*Plan: 01*
*Completed: 2026-04-16*
