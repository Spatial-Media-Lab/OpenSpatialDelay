---
phase: 03-personal-website
plan: 03
subsystem: ui
tags: [nextjs, tailwind, playwright, design-tokens, oklch, patreon, tdd]

# Dependency graph
requires:
  - phase: 03-personal-website/00
    provides: "Wave 0 validation infrastructure — tests/homepage-content.spec.ts extended here with 2 new D-12 colour assertions (regal-lavender positive + SectionLabel rose scope guard)."
  - phase: 03-personal-website/01
    provides: "app/page.tsx stable Hero + Features + DownloadCTA state (39122d0) with Hero Patreon ghost button at lines 160-166 untouched by 03-02, ready for recolour."
  - phase: 03-personal-website/02
    provides: "app/page.tsx AboutAndrew() contact-anchor migration (4de7977); 03-03 layers on top without overlap — AboutAndrew SectionLabel at line 478 left alone per D-12 scope guard."
provides:
  - "New --accent-regal CSS custom property (oklch(72% 0.15 290) ≈ #b49bd8) in globals.css, positioned immediately after --accent-rose per Phase 3 B-5 token slot."
  - "Patreon visual identity moved from rose-pink to light regal lavender at all 3 Patreon-branded call-sites: PatreonCTA section bg + button text (app/page.tsx lines 748, 774), Hero ghost button (app/page.tsx line 163), Nav ghost button (components/Nav.tsx lines 23-24)."
  - "2 new Playwright rendered-style assertions: (a) PatreonCTA computed backgroundColor matches the lab() form of oklch(72% 0.15 290) with ±1 channel tolerance, (b) AboutAndrew SectionLabel computed color still matches --accent-rose rgb(228,103,166) — scope-preservation regression guard."
  - "Empirical browser colour-space observation: Chromium (Playwright 1.x + Next.js 16.2.4) returns oklch()-sourced computed values in CIE lab() form, not sRGB rgb() form. Hex-sourced tokens remain in rgb() form."
affects: [03-04, 03-05, 03-06, 03-07, 03-08]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Wide-gamut CSS colour function rendering — oklch() source values arrive at Playwright's getComputedStyle().backgroundColor as lab() strings, not rgb(). Tests asserting against CSS custom properties defined with oklch() MUST match the lab() form (or use a tolerant wide-gamut matcher); tests against hex-sourced properties continue to use rgb(r,g,b) form."
    - "Two-sided colour-regression pattern — a token-swap plan pairs (a) a positive rendered-style assertion on the new token surface with (b) a negative/scope-guard rendered-style assertion on a retained surface of the old token. Catches drift in both directions: rename leaks (guard fails) and incomplete swaps (positive fails)."
    - "Post-measurement regex tightening — plan authorises the executor to tighten the rendered-style regex ONCE after first empirical measurement (Plan Task 2 Step E). Measured value and final regex bounds recorded in SUMMARY for audit. Explicitly forbids weakening to /rgb\\(.+\\)/ — that defeats the D-12 guard."

key-files:
  created:
    - ".planning/phases/03-personal-website/03-03-SUMMARY.md"
    - ".planning/phases/03-personal-website/deferred-items.md"
  modified:
    - "../andrewrahman-com/app/globals.css (new --accent-regal token at line 28)"
    - "../andrewrahman-com/app/page.tsx (4 occurrences of var(--accent-rose) → var(--accent-regal) across Hero ghost button line 163 + PatreonCTA section bg line 748 + CTA button text line 774; AboutAndrew SectionLabel line 478 UNCHANGED per D-12 scope guard)"
    - "../andrewrahman-com/components/Nav.tsx (2 occurrences of var(--accent-rose) → var(--accent-regal) in Patreon ghost button lines 23-24)"
    - "../andrewrahman-com/tests/homepage-content.spec.ts (2 new D-12 tests; regex form adjusted from rgb() to lab() after first Playwright measurement)"

key-decisions:
  - "Kept --accent-rose token in globals.css alongside the new --accent-regal token. D-12 is a scoped recolour of 3 Patreon call-sites, NOT a token rename. AboutAndrew's SectionLabel index='05 · Me' + the AboutAndrew radial-gradient rgba(228,103,166,0.12) both remain rose per UI-SPEC §Accent reserved-for list."
  - "Did NOT add an @theme inline bridge entry --color-regal-osd: var(--accent-regal). Grep confirmed zero consumers of regal-osd or bg-regal Tailwind utilities in app/ or components/; the 3 Patreon call-sites all use inline style={{ ... }} with var(--accent-regal), so a Tailwind bridge entry would be dead code. Plan Task 1 action explicitly authorised this skip."
  - "Post-measurement regex tightening (Plan Task 2 Step E): initial assertion expected rgb(180,155,216) for oklch(72% 0.15 290). Chromium actually returns lab(65.5944 24.778 -50.7219) because oklch() is a wide-gamut source. Regex tightened to /^lab\\(\\s*6[4-6]\\.\\d+\\s+2[3-5]\\.\\d+\\s+-5[01]\\.\\d+\\s*\\)$/ — ±1 channel tolerance guards D-12 intent: any drift back to --accent-rose returns rgb(228,103,166), which cannot match a lab(...) regex."

patterns-established:
  - "When a plan introduces a new CSS custom property using a wide-gamut colour function (oklch, color(display-p3 ...), lch, lab), Playwright rendered-style assertions MUST measure the computed value empirically before committing regex bounds. Chromium canonicalises wide-gamut sources to lab(), not rgb()."
  - "Scope-guard tests are written alongside swap tests. Any scoped recolour (vs token rename) gets a dedicated Playwright test asserting the retained surfaces still render the OLD token's computed colour. Prevents the 'overzealous find-and-replace' regression."
  - "Out-of-scope RED tests from earlier waves (Plan 03-00 scaffolded 6 failing tests waiting on Waves 2+) are logged to deferred-items.md, not fixed. Plan 03-03 ran the full npx playwright test and left the 6 unrelated failures alone per GSD SCOPE BOUNDARY rule."

requirements-completed: [WEB-01]

# Metrics
duration: ~3min
completed: 2026-04-16
---

# Phase 3 Plan 03: Patreon Recolour (--accent-rose → --accent-regal, Scoped 3-Site Swap) Summary

**Added new `--accent-regal` token (oklch(72% 0.15 290) ≈ #b49bd8) and swapped 3 Patreon-branded call-sites (PatreonCTA section + Hero ghost button + Nav ghost button) from --accent-rose to --accent-regal. AboutAndrew SectionLabel retains --accent-rose per D-12 scope guard; two-sided Playwright rendered-style regression (positive lab() match on the Patreon bg + negative rgb() guard on the rose label).**

## Performance

- **Duration:** ~3 min (162 s) — start 2026-04-16T22:17:42Z, end 2026-04-16T22:20:24Z
- **Tasks:** 2 of 2 complete (Task 1 auto, Task 2 tdd)
- **Files modified:** 5 (4 in sibling repo `../andrewrahman-com/` + 1 new SUMMARY + 1 new deferred-items log here)
- **Tests added:** 2 (1 positive D-12 + 1 scope-guard D-12)
- **Content-regression test totals after this plan:** 24 passed (15 homepage-content + 9 privacy-content)

## Accomplishments

- **Token added:** `--accent-regal: oklch(72% 0.15 290); /* ≈ #b49bd8 — light regal lavender; Patreon CTA only; Phase 3 B-5 */` at `app/globals.css:28`, immediately after `--accent-rose: #e467a6;` on line 27. No other `:root` tokens touched. No `@theme inline` bridge entry (zero existing Tailwind consumers per pre-commit grep).
- **3 Patreon call-sites recoloured** (6 string occurrences total across 2 files):
  - `app/page.tsx:163` — Hero ghost button `borderColor` + `color` (2 occurrences on one line).
  - `app/page.tsx:748` — PatreonCTA `<section>` `background` (1 occurrence).
  - `app/page.tsx:774` — PatreonCTA "Support on Patreon →" button `color` (1 occurrence).
  - `components/Nav.tsx:23-24` — Nav Patreon ghost button `borderColor` + `color` (2 occurrences across 2 lines).
- **Scope preserved:** `app/page.tsx:478` (`<SectionLabel index="05 · Me" accent="var(--accent-rose)" />` inside AboutAndrew) UNCHANGED. AboutAndrew radial-gradient numeric literal `rgba(228,103,166,...)` also UNCHANGED.
- **Two-sided Playwright regression landed** in `tests/homepage-content.spec.ts`:
  - `test('Patreon CTA uses the new regal-lavender token (D-12)')` — GREEN against `lab(65.5944 24.778 -50.7219)` with ±1 channel tolerance.
  - `test('AboutAndrew SectionLabel retains --accent-rose (D-12 scope guard)')` — GREEN against `rgb(228, 103, 166)` (unchanged from pre-plan measurement).
- **All in-scope tests green:** 24 passed combined (15 homepage-content + 9 privacy-content). 6 unrelated failures from Plan 03-00's Wave-0 scaffolding (a11y + asset-existence) logged to `deferred-items.md` — pre-existing, waiting on Waves 2+, NOT regressions caused by this plan.

## Task Commits

Each task was committed atomically in the sibling site repo `../andrewrahman-com/`:

1. **Task 1: Add --accent-regal token** — `fe74c5b` (feat) — `feat(03-03): add --accent-regal token (oklch(72% 0.15 290) ≈ #b49bd8)`
2. **Task 2 (RED): Playwright D-12 assertions + scope guard** — `3c2bb7e` (test) — `test(03-03): add regal-lavender + rose-scope-guard assertions (RED)`
3. **Task 2 (GREEN): 3-site recolour + regex tightening** — `7596f25` (feat) — `feat(03-03): recolour 3 Patreon sites to --accent-regal (GREEN)`

**Plan metadata:** pending — created by final-commit step below (SUMMARY.md + STATE.md + ROADMAP.md + deferred-items.md in the plugin repo).

## Files Created/Modified

### Sibling repo `/Users/andrewrahman/conductor/repos/andrewrahman-com/`

- `app/globals.css` — 1 line inserted at line 28 (new `--accent-regal` token + inline comment documenting sRGB approximation + Phase 3 B-5 slot provenance). Committed in `fe74c5b`.
- `app/page.tsx` — 4 character-level `var(--accent-rose)` → `var(--accent-regal)` swaps on lines 163 (×2), 748 (×1), 774 (×1). AboutAndrew line 478 untouched. Committed in `7596f25`.
- `components/Nav.tsx` — 2 character-level `var(--accent-rose)` → `var(--accent-regal)` swaps on lines 23 and 24. Committed in `7596f25`.
- `tests/homepage-content.spec.ts` — 2 tests appended inside the existing `homepage content` describe block (file grew from 105 → 123 lines). Regex form tightened from initial rgb-based assertion to final lab-based assertion after first empirical measurement. Committed in `3c2bb7e` (initial RED) and `7596f25` (regex refinement landed with GREEN source).

### Plugin repo `/Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/`

- `03-03-SUMMARY.md` (NEW, this file).
- `deferred-items.md` (NEW — logs 6 out-of-scope RED tests from Plan 03-00 Wave-0 scaffolding; waits on Waves 2+).

## Decisions Made

- **D-12 is a scoped recolour, not a token rename.** `--accent-rose` stays in `globals.css` alongside the new `--accent-regal`. Plan/UI-SPEC §Accent reserved-for list pins rose usage to AboutAndrew SectionLabel and radial-gradient (D-15 will later replace that div with a headshot, so the gradient rose goes away organically, but not in this plan).
- **No `@theme inline` bridge for `--color-regal-osd`.** Pre-commit grep confirmed zero Tailwind utility-class consumers of `regal-osd` or `bg-regal` in `app/` or `components/`. All 3 Patreon call-sites use inline `style={{ ... }}` with `var(--accent-regal)`. Adding a bridge entry would be dead code and drift from the established single-token-to-utility-alias convention.
- **Regex form is `lab(...)` not `rgb(...)`.** Initial plan anticipated `rgb(180, 155, 216)` for `oklch(72% 0.15 290)`. First empirical measurement (Next.js 16.2.4 + Chromium via Playwright) returned `lab(65.5944 24.778 -50.7219)` — Chromium canonicalises wide-gamut source values to lab() in `getComputedStyle()`. Per Plan Task 2 Step E, executor updated the regex once with tight ±1 channel bounds. The D-12 guard remains strict: a drift back to `--accent-rose` would produce `rgb(228,103,166)`, which cannot match the `^lab\(...\)$` regex.
- **6 failing tests from Plan 03-00 Wave-0 scaffolding NOT fixed.** `tests/a11y.spec.ts` (3 RED tests waiting on Plan 03-06) and `tests/asset-existence.spec.ts` (3 RED tests waiting on Plans 03-04/03-05) are pre-existing future-wave gates, not regressions from 03-03. Logged to `deferred-items.md` per GSD SCOPE BOUNDARY rule.

## Test Pass Count

- `tests/homepage-content.spec.ts`: **15 passed** (13 pre-existing from Plans 01/02 + 2 new D-12 assertions from Plan 03).
- `tests/privacy-content.spec.ts`: **9 passed** (unchanged from Plan 02).
- **Combined end-of-plan content regression run: 24 passed in 2.3s (2 workers).**
- Full `npx playwright test` at end-of-plan: 43 passed, 6 failed (failures are Plan 03-00 Wave-0 RED tests waiting on later waves — see `deferred-items.md`).

## Empirical Measurements (Task 2 Step 0)

Captured for audit + future Phase 3 plans that author rendered-style assertions:

| Element                                 | CSS source                   | Chromium `getComputedStyle()` value |
| --------------------------------------- | ---------------------------- | ----------------------------------- |
| PatreonCTA `<section>` background       | `oklch(72% 0.15 290)`        | `lab(65.5944 24.778 -50.7219)`      |
| AboutAndrew SectionLabel "05 · Me" text | `#e467a6` (via `--accent-rose`) | `rgb(228, 103, 166)`            |

Channel tolerances applied in the final regex: L ±1 (64–66), a ±1 (23–25), b ±1 (-51 to -50 inclusive). Tightness chosen so any drift back to rose or to a non-intended purple-adjacent token immediately fails the assertion.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 — Bug] Playwright regex assumed rgb() form for oklch()-sourced computed value**

- **Found during:** Task 2 Step D (first `npm test` run after source swaps landed).
- **Issue:** The RED assertion in `3c2bb7e` used `rgb(\s*1[6-9]\d\s*,\s*1[3-7]\d\s*,\s*2[0-2]\d\s*)`. Chromium returned `lab(65.5944 24.778 -50.7219)` — the regex failed. Without a fix the test would stay RED and the plan could not GREEN.
- **Fix:** Per Plan Task 2 Step E (explicit executor authority to adjust regex once post-measurement), replaced the regex with `^lab\(\s*6[4-6]\.\d+\s+2[3-5]\.\d+\s+-5[01]\.\d+\s*\)$`. The `^...$` anchoring + lab() literal + tight channel bounds preserve D-12 intent: any drift back to `--accent-rose` produces `rgb(228,103,166)` which cannot match.
- **Why Rule 1 (not Rule 4):** This is a bug in the test assertion's regex, not an architectural change. The plan explicitly anticipated this case and pre-authorised the adjustment.
- **Files modified:** `../andrewrahman-com/tests/homepage-content.spec.ts` (lines 107-113 block).
- **Verification:** 2 D-12 tests green in the Task 2 GREEN commit; full homepage-content suite 15/15 green.
- **Committed in:** `7596f25` (Task 2 GREEN commit, explicit note in commit body).

### Scope-boundary note

The 6 failing tests from Plan 03-00 Wave-0 scaffolding (`tests/a11y.spec.ts` × 3 + `tests/asset-existence.spec.ts` × 3) were NOT fixed. They are pre-existing, unrelated to this plan's Patreon recolour, and wait on Plans 03-04, 03-05, 03-06. Logged to `.planning/phases/03-personal-website/deferred-items.md` per GSD SCOPE BOUNDARY rule.

### Acceptance-criterion wording note

Plan Task 2 acceptance criterion `grep -c "var(--accent-regal)" app/page.tsx returns at least 4` uses `grep -c` which counts matching LINES, not OCCURRENCES. The Hero ghost button has both `borderColor` and `color` on a single line (line 163: `style={{ borderColor: 'var(--accent-regal)', color: 'var(--accent-regal)' }}`), so `grep -c` returns 3 (lines 163, 748, 774), while `grep -o ... | wc -l` returns 4 (the intended occurrence count). Semantic intent satisfied: 4 Hero/PatreonCTA swaps made per the plan's narrative. No action needed beyond this audit note.

---

**Total deviations:** 1 auto-fixed (Rule 1 — regex form bug)
**Impact on plan:** Single-line test adjustment, explicitly pre-authorised by the plan, documented with audit trail. Zero scope creep. Zero source-code deviations.

## Issues Encountered

- **CSS wide-gamut canonicalisation** — Chromium returns `oklch()` sources as `lab()` in computed style, not `rgb()`. Handled with pre-authorised regex refinement. Discovery captured in `tech-stack.patterns` for future plans that author rendered-style assertions.
- **Read-tool caching quirk** — after the site repo was already partially touched (globals.css + tests committed earlier), the Read tool returned "File unchanged — refer to earlier result" on re-reads. Worked around by using Grep with line numbers for inspection; Edit tool ran correctly against the cached-but-up-to-date files. No correctness impact.
- **Pre-existing static-server hazard** (Plan 01/02 documented) — `npx serve out -l 3000` workers stay alive between `npm test` runs. Killed stale workers with `pkill -f "serve out"` before the final build+test cycle. Standard Phase 3 hygiene step.

## User Setup Required

None — all automation ran without external service configuration.

## Next Phase Readiness

- **Plan 03-04 (next wave, first Wave-2 plan — REAPER hero screenshot + human checkpoint)** is unblocked. 03-03's token addition + recolour are self-contained inside Wave 1. 03-04 introduces a human checkpoint (REAPER screenshot capture) per the phase plan graph.
- **Plan 03-06 (a11y audit)** — the new `--accent-regal` token pairs with `--bg-void` on the PatreonCTA section. axe will evaluate this pairing for WCAG AA contrast. oklch(72% 0.15 290) is L≈72, very bright, so white-text-on-regal should pass comfortably; light-text on regal may need reconfirmation when the a11y sweep runs. Not pre-evaluated here per plan scope.
- **Plan 03-00 Wave-0 RED tests** — the 6 failing tests in `deferred-items.md` will turn GREEN as Plans 03-04, 03-05, 03-06 land their work. No blocker for 03-03 completion.

## Self-Check: PASSED

Verified post-write:

- `test -f /Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/03-03-SUMMARY.md` — FOUND (written via Write tool).
- `test -f /Users/andrewrahman/conductor/repos/openspatialdelay/.planning/phases/03-personal-website/deferred-items.md` — FOUND.
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep fe74c5b` — FOUND: `feat(03-03): add --accent-regal token (oklch(72% 0.15 290) ≈ #b49bd8)`.
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 3c2bb7e` — FOUND: `test(03-03): add regal-lavender + rose-scope-guard assertions (RED)`.
- `git -C /Users/andrewrahman/conductor/repos/andrewrahman-com log --oneline | grep 7596f25` — FOUND: `feat(03-03): recolour 3 Patreon sites to --accent-regal (GREEN)`.
- AboutAndrew `--accent-rose` retention verified: `grep -c 'accent="var(--accent-rose)"' app/page.tsx` returns 1 (SectionLabel "05 · Me"); rose token present in globals.css line 27.
- Final content-regression run: **24 passed** (15 homepage + 9 privacy).

---
*Phase: 03-personal-website*
*Plan: 03*
*Completed: 2026-04-16*
